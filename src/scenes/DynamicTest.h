#pragma once

#include "Scene.h"
#include "Renderer.h"
#include "Shader.h"

#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "Texture.h"
#include "shapes/Shapes.h"

namespace scene{
    class DynamicTest : public Scene{
    private:
        float vertices[32] = {
            -0.5f, -0.5f,   0.8f, 0.2f, 0.6f, 1.0f,     0.0f, 0.0f,
            -0.5f, 0.5f,    0.8f, 0.2f, 0.6f, 1.0f,     0.0f, 1.0f,
            0.5f, -0.5f,    0.8f, 0.2f, 0.6f, 1.0f,     1.0f, 0.0f,
            0.5f, 0.5f,     0.8f, 0.2f, 0.6f, 1.0f,     1.0f, 1.0f
        };

        VertexBuffer* vb;
        VertexBufferLayout* layout;
        IndexBuffer* ib;
        VertexArray* va;
        Shader* shader;
        Shader* shapeShader;


        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 model;

        glm::vec3 translation;
        float rotation;
        float scale;
        
        glm::mat4 mvp;

    public:
        DynamicTest(GameManager* manager);
        ~DynamicTest();

        void OnUpdate(float deltaTime) override;
        void OnRender() override;
        void OnImGuiRender() override;
    };
}