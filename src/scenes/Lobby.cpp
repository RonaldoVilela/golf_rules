#include "Lobby.h"
#include "GameManager.h"
#include "scenes/Match.h"

#include "imgui/imgui.h"
#include <thread>
#include <winsock2.h>

namespace scene{
    Lobby::Lobby(GameManager *manager): Scene(manager)
    {
        everyoneReady = false;
        onlinePlayers = 0;

        serverPort = 0;

        float vertices[16] = {
            -0.5f, -0.5f,   0.0f, 0.0f,
            -0.5f, 0.5f,    0.0f, 1.0f,
            0.5f, -0.5f,    1.0f, 0.0f,
            
            0.5f, 0.5f,     1.0f, 1.0f
        };

        unsigned int indices[6] = {
            0, 1, 2,
            1, 2, 3
        };

        //texture = new Texture("res/textures/texture.png");
        //texture->Bind();

        vb = new VertexBuffer(nullptr, sizeof(float)*16, GL_DYNAMIC_DRAW);
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
                    }
                    break;

                case PLAYER_DISCONNECTION_EVENT:
                    manager->connected_players.erase(event->m_flag);
                    break;

                case START_MATCH_EVENT:
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
        if(manager->player.onlineStatus == SV_CLIENT){
            manageServerEvents();
        }
    }
    void Lobby::OnRender()
    {
        GameManager::useShader(GM_FONT_SHADER);
        

        manager->ui_FrameBuffer.Bind();

        //texture->Bind();
        GLCall(glActiveTexture(GL_TEXTURE0));
        GLCall(glBindTexture(GL_TEXTURE_2D, manager->font.textureID));
    

        GameManager::activeShader->SetUniformMat4f("u_MVP", manager->windowProjection);
        //Renderer::Draw(*va, *ib, *GameManager::activeShader);
        Renderer::drawString(std::to_string(serverPort),10, 30,{1.0f, 1.9f, 0.95f, 1.0f});

        manager->ui_FrameBuffer.Unbind();
    }

    void Lobby::OnImGuiRender()
    {
        ImGui::Begin("Lobby placeHolder");

        if(ImGui::InputText("NickName", manager->player.m_name, 24)){
            //std::cout << "changed name";
        }

        if(ImGui::Button("Host server")){
            serverPort = manager->hostServer();
        }

        ImGui::InputText("ServerIp", serverIp, 15);
        ImGui::InputInt("Port", &serverPort);

        if(ImGui::Button("Join Server")){
            manager->joinServer(serverIp, serverPort);
        }

        if(ImGui::Button("Disconnect")){
            manager->disconnect();
            serverPort = 0;
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

        ImGui::Text("connected players: %d", manager->connected_players.size());

        for(auto p : manager->connected_players){
            ImGui::Text("[%d] %s",p.first, p.second.m_name);
        }

        if(ImGui::Button("start game")){
            if(manager->player.onlineStatus == SV_HOSTING){
                AbstractEvent* newEvent = new AbstractEvent{START_MATCH_EVENT};
                manager->sendEvent(newEvent);
                delete newEvent;
            }
            if(manager->player.onlineStatus != SV_CLIENT){

                //if none of the loading functions returns an error...
                if( !(((scene::Match *)manager->scenes["match"])->loadCourse("db_testcourse") ||
                ((scene::Match *)manager->scenes["match"])->loadMap("Untitled")))
                {
                    manager->changeScene("match");
                }
            }
        }
        ImGui::End();
    }

    void Lobby::HandleEvents(GLFWwindow *window)
    {
    }
};