#include "GameManager.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include "GLFW/glfw3.h"

#include <glm/gtc/matrix_transform.hpp>
#include "scenes/Lobby.h"
#include "scenes/Match.h"

// ============================================== //

float GameManager::scrollFactor = 0;
Player GameManager::player;
int GameManager::lastConnectionId = 0;
std::map<int, Player> GameManager::connected_players;
std::queue<std::array<char, 256>> GameManager::eventList;

std::vector<textLog> GameManager::console_cache;

std::vector<Shader*> GameManager::shaders;
Shader* GameManager::activeShader;

FontAtlas GameManager::font;
VertexBuffer* GameManager::textVertexBuffer;
IndexBuffer* GameManager::textIndexBuffer;
VertexArray* GameManager::textVertexArray;

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

    screenResolutions.push_back({
        //TODO: Especify the dimensions of the resolutions
    });


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

    /*
        Load a dynamic vertexBuffer, wich can and will be used to store texts letter's vertices, for
        later text rendering operations.
    */
    textVertexBuffer = new VertexBuffer(nullptr ,sizeof(float)*16*1000 ,GL_DYNAMIC_DRAW);
    textIndexBuffer = new IndexBuffer(nullptr,6*1000);
    textVertexArray = new VertexArray();

    VertexBufferLayout layout;
    layout.Push<float>(2);
    layout.Push<float>(2);
    
    textVertexArray->AddBuffer(*textVertexBuffer, layout);

    /*
        TODO: add the damn explanation
    */
    float screenVertex[16] = {
        -1.0f, 1.0f,    0.0f, 1.0f,
        1.0f, 1.0f,     1.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,

        1.0f, -1.0f,    1.0f, 0.0f
    };

    screenVertexArray.AddBuffer(screenVertexBuffer, layout);

    screenVertexBuffer.Bind();
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*16, screenVertex));
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

    
    delete textVertexBuffer;

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
        actualScene = scenes[name];
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

void GameManager::drawUiFrameBuffer()
{

    ui_FrameBuffer.BindTexture();

    useShader(GM_TEXTURE_SHADER);
    activeShader->SetUniformMat4f("u_MVP", projection);
    Renderer::Draw(screenVertexArray, screenIndexBuffer, *activeShader);
}
