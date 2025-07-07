#pragma once

#include "Scene.h"
#include "Renderer.h"
#include "Shader.h"

#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "Texture.h"
#include "box2d/box2d.h"
#include <map>
#include <vector>

struct ServerInfo;
namespace scene{
    class ServerList : public Scene{
        private:
            char server_name[30] = {};

            char serverIp[15] = {};
            int serverPort = 0;

            static std::vector<ServerInfo> local_servers; //WARNING: This variable is not being used yet
            
        public:
            ServerList(GameManager* manager);
            ~ServerList();
            
            void start() override;
            void OnUpdate(float deltaTime) override;
            void OnRender() override;

            void OnImGuiRender() override;
            void HandleEvents(GLFWwindow* window) override;
    };
}