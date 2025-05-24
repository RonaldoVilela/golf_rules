#pragma once


#include "Scene.h"
#include "Renderer.h"
#include "Shader.h"

#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "Texture.h"

namespace scene{
    class QuadBashTest : public Scene{
    private:

        VertexBuffer* vb;
        VertexBufferLayout* layout;
        IndexBuffer* ib;
        VertexArray* va;
        Shader* shader;

        Texture* tex1;
        Texture* tex2;

        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 model;

        glm::vec3 translation;
        float rotation;
        float scale;
        
        glm::mat4 mvp;

    public:
        QuadBashTest(GameManager* manager);
        ~QuadBashTest();

        void OnRender() override;

        void OnImGuiRender() override;
    };
}