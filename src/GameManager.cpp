#include "GameManager.h"

#include <fstream>
#include <iostream>
#include <thread>
#include "GLFW/glfw3.h"

#include <glm/gtc/matrix_transform.hpp>
#include "scenes/Lobby.h"
#include "scenes/Match.h"
#include "scenes/ServerList.h"

// ============================================== //

float GameManager::scrollFactor = 0;

ServerInfo GameManager::actual_server;
Player GameManager::player;
int GameManager::lastConnectionId = 0;
std::map<int, Player> GameManager::connected_players;
std::queue<std::array<char, 256>> GameManager::eventList;

std::vector<textLog> GameManager::console_cache;

std::vector<Shader*> GameManager::shaders;
Shader* GameManager::activeShader;

FontAtlas GameManager::font;

// ============================================== //

void GameManager::updateScreenSize(GLFWwindow* window,int width, int height)
{
    glfwSetWindowSize(window,width, height);
    screenSize = {width, height};
    screenBounds = {-2.0f, 2.0f, -1.5f, 1.5f};
    windowProjection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

    float screenVertex[16] = {
        screenBounds.left, screenBounds.top,        0.0f, 1.0f,
        screenBounds.right, screenBounds.top,       1.0f, 1.0f,
        screenBounds.left, screenBounds.bottom,     0.0f, 0.0f,

        screenBounds.right, screenBounds.bottom,    1.0f, 0.0f
    };

    screenVertexBuffer.Bind();
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*16, screenVertex));

    ui_FrameBuffer.resize(width,height);
}

void GameManager::updateMousePos(GLFWwindow *window)
{
    double actualMousePos[2];
    glfwGetCursorPos(window, &actualMousePos[0], &actualMousePos[1]);
    mouseNormPos = windowProjection * glm::vec4(actualMousePos[0], actualMousePos[1], 0.0f, 1.0f);
    mouseNormPos.x *= screenBounds.right;
    mouseNormPos.y *= screenBounds.top;
}

void GameManager::handleEvents(GLFWwindow *window)
{
    updateMousePos(window);
    static bool clicked = false;
    if(glfwGetKey(window, '`') == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard && !clicked){
        consoleOpen = !consoleOpen;
        clicked = true;
    }
    if(glfwGetKey(window,'`') == GLFW_RELEASE && !ImGui::GetIO().WantCaptureKeyboard){
        clicked = false;
    }
}

void GameManager::scroll_callback(GLFWwindow *window, double offsetX, double offsetY)
{
    scrollFactor = offsetY*2;
}

GameManager::GameManager(GLFWwindow* window) : 
window(window),ui_FrameBuffer(800,600), screenVertexBuffer(nullptr, sizeof(float)*16, GL_DYNAMIC_DRAW), screenIndexBuffer(screenVexIndicies, 6)
{
    font = LoadFont("res/fonts/ROCK.TTF",32);
    strcpy(actualCommand, "");
    loadConsoleCommands();

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(result != 0){
        std::cerr << "WSAStartup failed: " << result << std::endl;
        GM_LOG("WSAStartup failed: " + std::to_string(result) + " <- error code", LOG_ERROR);
        return;
    }

    // Screen ratios and resolutions ------------ //
    // 4:3 - [0]
    screenResolutions.push_back(
        {{640, 480}, {800, 600}, {1024, 768}, {1280, 960}}
    );
    // 16:10 - [1]
    screenResolutions.push_back(
        {{1280, 800}, {1440, 900}, {1680, 1050}}
    );
    // 16:9 - [2]
    screenResolutions.push_back(
        {{854, 480}, {1280, 720}, {1600, 900}, {1920, 1080}}
    );
    // ---------------------------------------- //


    projection = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, -1.0f, 1.0f);

    // Load default shapes for debug rendering.
    circle = new shapes::Circle();
    square = new shapes::Square();
    line = new shapes::Line();

    // Load and store the game shaders.
    shaders.push_back(new Shader("res/shaders/basicShader.shader"));    // 0: Used for rendering shapes and lines.
    shaders.push_back(new Shader("res/shaders/fontShader.shader"));     // 1: Used for text rendering.
    shaders.push_back(new Shader("res/shaders/textureShader.shader"));  // 2: Used for rendering textures.
    // -----------------------

    player.onlineStatus = SV_DISCONNECTED;

    scenes.insert(std::make_pair("lobby",new scene::Lobby(this)));
    scenes.insert(std::make_pair("match",new scene::Match(this)));
    scenes.insert(std::make_pair("server_list",new scene::ServerList(this)));

    VertexBufferLayout layout;
    layout.Push<float>(2);
    layout.Push<float>(2);

    /*
        TODO: add the damn explanation
    */
    float screenVertex[16] = {
        -1.0f, 1.0f,    0.0f, 1.0f,
        1.0f, 1.0f,     1.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,

        1.0f, -1.0f,    1.0f, 0.0f
    };

    screenVertexBuffer.Bind();
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*16, screenVertex));
    screenVertexArray.AddBuffer(screenVertexBuffer, layout);

    
}

GameManager::~GameManager()
{
    if(player.onlineStatus != SV_DISCONNECTED){disconnect();}

    // Check if Winsock was initialized, if yes, terminate Winsock
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s != INVALID_SOCKET){
        closesocket(s);
        WSACleanup();
    }

    //delete shapes
    delete circle;
    delete square;
    delete line;

    // clear scenes
    actualScene = nullptr;
    for(auto s: scenes){
        delete s.second;
    }
}
void GameManager::addShader(Shader* shader)
{
    shaders.push_back(shader);
}

void GameManager::changeScene(std::string name)
{
    if(scenes.find(name) != scenes.end()){
        if(actualScene != nullptr){
            actualScene->unload();
        }
        actualScene = scenes[name];
        actual_scene_name = name;
        actualScene->start();
        return;
    }
    std::cout << "Warning: there is no scene named '" << name << "'\n";
}

void GameManager::useShader(int index)
{
    if(shaders.size() <= index){
        std::cout << "there is no shader in index " << index << ".\n";
        return;
    }
    shaders[index]->Bind();
    activeShader = shaders[index];
}

int GameManager::getBinaryImages(std::string dat_file, std::map<std::string, ImageData> *image_data_buffer)
{
    std::ifstream file("res/textures/"+ dat_file, std::ios::binary);
    std::cout << "now loading dat file... \n";
    if(file.is_open()){
        int image_number = 0;
        file.read((char*)&image_number, sizeof(int));

        for(int i = 0; i < image_number; i++){
            ImageData image_data;

            int name_size = 0;
            file.read((char*)&name_size, sizeof(int));
             // image name lenght

            char* name_buffer = (char*)malloc(name_size + 1);
            file.read(name_buffer, name_size); // image name

            name_buffer[name_size] = '\0';
            std::string image_name(name_buffer);

            free(name_buffer);


            file.read((char*)&image_data.width, sizeof(int)); // width
            file.read((char*)&image_data.height, sizeof(int)); // height
            file.read((char*)&image_data.m_bytesPerPixel, sizeof(int)); // bytes per pixel
            file.read((char*)&image_data.m_byteSize, sizeof(int)); // total byte size
            image_data.m_data = (unsigned char*)malloc(image_data.m_byteSize);
            file.read((char*)image_data.m_data, image_data.m_byteSize); // data

            image_data_buffer->insert(std::make_pair(image_name, image_data));
            
        }
        file.close();
    }else{
        std::cout << "Error: couldn't locate the .dat file: "<< dat_file << "\n";
        return 1;
    }
    return 0;
}

void GameManager::drawUiFrameBuffer()
{

    ui_FrameBuffer.BindTexture();

    useShader(GM_TEXTURE_SHADER);
    activeShader->SetUniformMat4f("u_MVP", projection);
    Renderer::Draw(screenVertexArray, screenIndexBuffer, *activeShader);
}


