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

struct ImageData;

namespace scene{
    
    struct LobbyEvent{
        int event_type;
        int event_flag1;
        int event_flag2;
        char event_string[30];
    };

    class Lobby: public Scene{
    private:
        bool everyoneReady;
        int onlinePlayers;

        char serverIp[15] = {};
        int serverPort = 0;

        //WARNING: temporary code -------- //
        std::map<std::string, ImageData> images;
        VertexBuffer* vb;
        VertexBufferLayout* layout;

        IndexBuffer* ib;

        VertexArray* va;
        Texture* texture;
        Texture* anon_texture;
        // ------------------------------- //

        glm::mat4 view;
        glm::mat4 model;

        glm::vec3 translation;
        float rotation;
        float scale;
        
        glm::mat4 mvp;
    public: 
        Lobby(GameManager* manager);
        ~Lobby();

        void manageServerEvents();

        void start() override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;

        void OnImGuiRender() override;
        void HandleEvents(GLFWwindow* window) override;

        
    };
}