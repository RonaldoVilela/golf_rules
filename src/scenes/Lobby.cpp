#include "Lobby.h"
#include "GameManager.h"
#include "scenes/Match.h"

#include "imgui/imgui.h"
#include <thread>
#include <winsock2.h>
#include <sstream>

namespace scene{
    void Lobby::serverUpdate()
    {
        std::ostringstream connection_event_buffer(std::ios::binary);
        int event_type = SERVER_INFO_UPDATE_EVENT;
        connection_event_buffer.write((char*)&event_type, sizeof(int));
        connection_event_buffer.write(GameManager::actual_server.getServerInfo(manager->connected_players.size()), 250);

        manager->sendEvent(connection_event_buffer.str().data());
    }

    Lobby::Lobby(GameManager *manager): Scene(manager)
    {
        everyoneReady = false;
        onlinePlayers = 0;

        float vertices[16] = {
            -2.72f, -1.53f,   0.0f, 0.0f,
            -2.72f, 1.53f,    0.0f, 1.0f,
            2.72f, -1.53f,    1.0f, 0.0f,
            
            2.72f, 1.53f,     1.0f, 1.0f
        };

        unsigned int indices[6] = {
            0, 1, 2,
            1, 2, 3
        };
        GameManager::getBinaryImages("defTextures.dat", &images);

        std::cout << "now printing images.." << "\n";
        for(auto i: images){
            std::cout << i.first << "\n"; 
        }
        texture = new Texture(images["lobby.png"].width, images["lobby.png"].height, images["lobby.png"].m_bytesPerPixel, images["lobby.png"].m_data);
        anon_texture = new Texture(images["specProf.png"].width, images["specProf.png"].height, images["specProf.png"].m_bytesPerPixel, images["specProf.png"].m_data);
        //texture = new Texture("res/textures/texture.png");
        texture->Bind();
        std::cout << "texture created" << "\n"; 

        vb = new VertexBuffer(vertices, sizeof(float)*16, GL_STATIC_DRAW);
        layout = new VertexBufferLayout();
        layout->Push<float>(2);
        layout->Push<float>(2);

        ib = new IndexBuffer(indices, 6);

        va = new VertexArray();
        va->AddBuffer(*vb, *layout);

        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));

        translation = glm::vec3(0.0f, 0.0f, 0.0f);
        rotation = 0.0f;
        scale = 1.0f;

        model = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f)), translation);

        mvp = manager->projection * view * model;

        //--------------------;

        GameManager::useShader(GM_FONT_SHADER);
        GameManager::activeShader->SetUniformMat4f("u_MVP", mvp);
    }

    Lobby::~Lobby()
    {
        delete vb;
        delete layout;
        delete ib;
        delete va;
        delete texture;
    }

    void Lobby::manageServerEvents()
    {
        while(manager->eventList.size() > 0 && manager->player.onlineStatus == SV_CLIENT){

            std::array<char, 256> temp = manager->eventList.front();
            char eventBuffer[256];
            std::memcpy(eventBuffer, temp.data(), 256);

            AbstractEvent* event = (AbstractEvent*)eventBuffer;
            int eventType = event->eventType;
                
            switch(eventType){
                case SERVER_CLOSED_EVENT:
                        manager->disconnect();
                    break;
                case PLAYER_CONNECTION_EVENT: 
                    {
                        Player newPlayer;
                        strcpy(newPlayer.m_name, event->m_string);
                        newPlayer.connection_id = event->m_flag;
                        manager->connected_players.insert(std::make_pair(newPlayer.connection_id, newPlayer));
                        GM_LOG(std::string(newPlayer.m_name) + " joined the crew!");
                    }
                    break;
                
                case SERVER_INFO_UPDATE_EVENT:
                    GameManager::actual_server.setServerInfo(&eventBuffer[sizeof(int)]);
                    break;

                case PLAYER_DISCONNECTION_EVENT:
                    GM_LOG(std::string(manager->connected_players.at(event->m_flag).m_name) + " left the room");
                    manager->connected_players.erase(event->m_flag);
                    break;

                case START_MATCH_EVENT:
                    if( !(((scene::Match *)manager->scenes["match"])->loadCourse("db_testcourse") ||
                    ((scene::Match *)manager->scenes["match"])->loadMap("Untitled")))
                    {
                        manager->changeScene("match");
                    }else{
                        manager->disconnect();
                    }
                    break;

                case MATCH_MAP_RESPONSE:
                    // wait for the response:

                    std::cout << "Received the file names here :D \n";

                    int size = 0;
                    int buffer_position = sizeof(int);

                    memcpy(&size, &eventBuffer[buffer_position], sizeof(int)); // course name lenght
                    buffer_position += sizeof(int);

                    char* name_buffer = (char*)malloc(size + 1);
                    name_buffer[size] = '\0';

                    memcpy(name_buffer, &eventBuffer[buffer_position], size); // course name
                    buffer_position += size;

                    if(((scene::Match *)manager->scenes["match"])->loadCourse(name_buffer)){
                        manager->disconnect();
                        break;
                    }
                    std::cout << "loaded course \n";
                    free(name_buffer);

                    memcpy(&size, &eventBuffer[buffer_position], sizeof(int)); // hole name lenght
                    buffer_position += sizeof(int);

                    name_buffer = (char*)malloc(size + 1);
                    name_buffer[size] = '\0';

                    memcpy(name_buffer, &eventBuffer[buffer_position], size); // hole name
                    buffer_position += size;

                    if(((scene::Match *)manager->scenes["match"])->loadMap(name_buffer)){
                        manager->disconnect();
                        break;
                    }
                    std::cout << "Loaded map \n";

                    free(name_buffer);

                    manager->changeScene("match");
                    break;
            }
            manager->eventList.pop();
            std::cout << "Received one event here\n";
        };
    }

    void Lobby::start()
    {
        onlinePlayers = 0;
    }

    void Lobby::OnUpdate(float deltaTime)
    {
        if(manager->player.onlineStatus != SV_DISCONNECTED){
            manageServerEvents();
        }
    }
    void Lobby::OnRender()
    {
        GameManager::useShader(GM_FONT_SHADER);
        

        texture->Bind();
        GameManager::useShader(GM_TEXTURE_SHADER);
        GameManager::activeShader->SetUniformMat4f("u_MVP", mvp);
        Renderer::Draw(*va, *ib, *GameManager::activeShader);

        manager->ui_FrameBuffer.Bind();

        GLCall(glActiveTexture(GL_TEXTURE0));
        GLCall(glBindTexture(GL_TEXTURE_2D, manager->font.textureID));



        GameManager::useShader(GM_FONT_SHADER);
        GameManager::activeShader->SetUniformMat4f("u_MVP", manager->windowProjection);
        
        Renderer::drawString(manager->actual_server.m_name,10, 30,1.0f,(manager->actual_server.valid) ? glm::vec4{0.0f, 1.0f, 0.0f, 0.5f}:glm::vec4{0.5f, 0.5f, 0.5f, 0.5f});
        Renderer::drawString(std::to_string(manager->actual_server.m_port),10, 80,0.8f,{1.0f, 0.0f, 0.0f, 1.0f});

        anon_texture->Bind();
        GameManager::useShader(GM_TEXTURE_SHADER);
        GameManager::activeShader->SetUniformMat4f("u_MVP", manager->windowProjection);

        //Renderer::drawTextures(20, 20, 60, 60);
        
        int i = 0;
        for(auto p: manager->connected_players){
            Renderer::drawTextures(20, i*80 + (120), 60, 60);
            i++;
        }
        GameManager::useShader(GM_FONT_SHADER);
        GLCall(glActiveTexture(GL_TEXTURE0));
        GLCall(glBindTexture(GL_TEXTURE_2D, manager->font.textureID));
        
        i = 0;
        glm::vec4 blue = {0.0f, 0.0f, 1.0f, 1.0f};
        glm::vec4 red = {1.0f, 0.0f, 0.0f, 1.0f};

        for(auto p: manager->connected_players){
            glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};
            if(GameManager::actual_server.playerId_1 == p.first){color = blue;}
            if(GameManager::actual_server.playerId_2 == p.first){color = red;}

            Renderer::drawString(p.second.m_name, 100, i*80 + (160), 1.0f,color);
            i++;
        }

        manager->ui_FrameBuffer.Unbind();
    }

    void Lobby::OnImGuiRender()
    {
        ImGui::Begin("Lobby placeHolder");

        

        if(ImGui::Button("Disconnect")){
            manager->disconnect();
            manager->changeScene("server_list");
        }

        switch(manager->player.onlineStatus){
            case SV_DISCONNECTED: ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f},"Not Connected");
                break;
            case SV_HOSTING: ImGui::TextColored({1.0f, 1.0f, 0.0f, 1.0f},"Available for connections...");
                break;
            case SV_CLIENT: ImGui::TextColored({0.0f, 1.0f, 0.0f, 1.0f},"Connected to a server");
                break;
            default: ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f},"???");
        }

        if(manager->player.onlineStatus == SV_HOSTING){
            if(ImGui::InputInt("Player 1", &GameManager::actual_server.playerId_1,1,100,ImGuiInputTextFlags_EnterReturnsTrue) ||
            ImGui::InputInt("Player 2", &GameManager::actual_server.playerId_2,1,100,ImGuiInputTextFlags_EnterReturnsTrue)){
                serverUpdate();
            }
        }

        ImGui::Text("connected players: %d", manager->connected_players.size());

        for(auto p : manager->connected_players){
            ImGui::Text("[%d] %s",p.first, p.second.m_name);
            if(p.first == manager->player.connection_id){
                ImGui::SameLine();
                ImGui::Text("(you)");
            }
        }

        if(ImGui::Button("start game")){
            if(manager->player.onlineStatus == SV_HOSTING){
                AbstractEvent* newEvent = new AbstractEvent{START_MATCH_EVENT};
                manager->sendEvent(newEvent);
                delete newEvent;

                //if none of the loading functions returns an error...
                if( !(((scene::Match *)manager->scenes["match"])->loadCourse("db_testcourse") ||
                ((scene::Match *)manager->scenes["match"])->loadMap("Untitled")))
                {
                    manager->changeScene("match");
                    GameManager::actual_server.actual_course = "db_testcourse";
                }
            }
        }
        ImGui::End();
    }

    void Lobby::HandleEvents(GLFWwindow *window)
    {
    }
};