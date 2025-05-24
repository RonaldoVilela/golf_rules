#include "TextureTransformTest.h"
#include "Renderer.h"
#include "GameManager.h"
#include "GLFW/glfw3.h"
#include "imgui.h"

namespace scene{
    TextureTransformTest::TextureTransformTest(GameManager* manager) : Scene(manager)
    {
        texture = new Texture("res/textures/texture.png");
        texture->Bind();

        vb = new VertexBuffer(vertices, sizeof(vertices), GL_STATIC_DRAW);
        layout = new VertexBufferLayout();
        layout->Push<float>(2);
        layout->Push<float>(2);

        ib = new IndexBuffer(indices, 6);

        va = new VertexArray();
        va->AddBuffer(*vb, *layout);

        shader = new Shader("res/shaders/textureShader.shader");

        //Transformation

        projection = glm::ortho(-2.0f,2.0f, -1.5f, 1.5f, -1.0f, 1.0f);
        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));

        translation = glm::vec3(0.0f, 0.0f, 0.0f);
        rotation = 0.0f;
        scale = 1.0f;

        model = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f)), translation);

        mvp = projection * view * model;

        //--------------------;

        shader->Bind();
        shader->SetUniformMat4f("u_MVP", mvp);
    }
    TextureTransformTest::~TextureTransformTest()
    {
        delete vb;
        delete layout;
        delete ib;
        delete va;
        delete shader;
        delete texture;
    }

    void TextureTransformTest::OnUpdate(float deltaTime)
    {
    }
    void TextureTransformTest::OnRender()
    {
        shader->Bind();
        texture->Bind();
        
        model = glm::translate(glm::mat4(1.0f), translation)
        * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f,0.0f,1.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(scale,scale,1));

        mvp = projection * view * model;


        Renderer::Draw(*va, *ib, *shader);
        shader->SetUniformMat4f("u_MVP", mvp);
    }
    void TextureTransformTest::OnImGuiRender()
    {
        ImGui::Begin("Texture/vertex transforming");
        ImGui::SliderFloat2("Position", &translation.x, -2.0, 2.0f);
        ImGui::SliderFloat("Scale", &scale, 0.0f, 2.0f);
        ImGui::SliderFloat("Rotation", &rotation, -3.14f, 3.14f);
        ImGui::End();
    }
}