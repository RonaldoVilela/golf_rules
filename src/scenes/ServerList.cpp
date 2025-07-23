#include "ServerList.h"
#include "GameManager.h"
#include "scenes/Match.h"

#include "imgui/imgui.h"
#include <thread>
#include <winsock2.h>
#include <iphlpapi.h>


namespace scene{

    std::vector<ServerInfo> ServerList::local_servers;

    ServerList::ServerList(GameManager *manager) : Scene(manager)
    {
        
    }
    ServerList::~ServerList()
    {
    }

    void ServerList::start(){}
    void ServerList::OnUpdate(float deltaTime){}
    void ServerList::OnRender(){}

    void ServerList::OnImGuiRender(){
        //ImGui::ShowDemoWindow();
        ImGui::Begin("Server List", __null, ImGuiWindowFlags_NoCollapse);

        if(ImGui::InputText("NickName", manager->player.m_name, 24)){
            //std::cout << "changed name";
        }
        ImGui::InputText("Server name", server_name, 28);

        if(ImGui::Button("Host server")){
            serverPort = manager->hostServer(server_name);
            if(serverPort > 0){
                manager->changeScene("lobby");
            }
        }

        ImGui::InputText("ServerIp", serverIp, 15);
        ImGui::InputInt("Port", &serverPort);

        if(ImGui::Button("Join Server")){
            manager->joinServer(serverIp, serverPort);
        }

        if(ImGui::Button("Search server")){
            local_servers.clear();
            ServerInfo info = manager->searchServer();
            if(info.valid){
                GM_LOG("-- Response from the server: "+ info.m_name + " --");
                GM_LOG("Server address: "+ info.server_address);
                GM_LOG("Server port: "+ std::to_string(info.m_port));
                GM_LOG("Online players: "+ std::to_string(info.online_players) + "/x");
                if(info.actual_course.length() > 0){
                    GM_LOG("State: "+ info.actual_course);
                }else{
                    GM_LOG("State: On lobby...");
                }
                GM_LOG("--------------------");

                local_servers.push_back(info);
            }

        }
        
        ImGui::Text("Servers found:");

        for(int i = 0; i < local_servers.size(); i++){
            ImGui::BeginChild(local_servers[i].m_name.c_str(), ImVec2(220, 100), true);

            ImGui::Text(local_servers[i].m_name.c_str());
            ImGui::Text("Players online: %d/x", local_servers[i].online_players);
            if(ImGui::Button("Enter Server")){
                manager->joinServer(local_servers[i].server_address.c_str(), local_servers[i].m_port);
            }

            ImGui::EndChild();
        }

        ImGui::End();
    }
    void ServerList::HandleEvents(GLFWwindow* window){}
}