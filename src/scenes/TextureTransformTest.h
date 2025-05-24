#pragma once

#include "Scene.h"
#include "Renderer.h"
#include "Shader.h"

#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "Texture.h"

namespace scene{
    class TextureTransformTest : public Scene{
    private:
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

        VertexBuffer* vb;
        VertexBufferLayout* layout;

        IndexBuffer* ib;

        VertexArray* va;

        Shader* shader;
        Texture* texture;

        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 model;

        glm::vec3 translation;
        float rotation;
        float scale;
        
        glm::mat4 mvp;

    public:
        TextureTransformTest(GameManager* manager);
        ~TextureTransformTest();

        void OnUpdate(float deltaTime) override;
        void OnRender() override;

        void OnImGuiRender() override;
    };
}
