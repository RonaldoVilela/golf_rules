#include "QuadBashTest.h"
#include "Renderer.h"
#include "GameManager.h"
#include "GLFW/glfw3.h"

#include "imgui.h"

namespace scene{
    QuadBashTest::QuadBashTest(GameManager* manager) : Scene(manager)
    {
        tex1 = new Texture("res/textures/texture.png");

        tex2 = new Texture("res/textures/thing.png");
        
        tex1->Bind(0);
        tex2->Bind(1);

        struct Vertex{
            float position[2];
            float color[4];
            float texCoord[2];
            float texId;
        };

        float vertices[] = {
            -1.5f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f,     0.0f, 0.0f,     0.0f,
            -1.5f, 0.5f,    1.0f, 0.0f, 0.0f, 1.0f,     0.0f, 1.0f,     0.0f,
            -0.5f, -0.5f,   1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 0.0f,     0.0f,
            -0.5f, 0.5f,    1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 1.0f,     0.0f,

            0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 0.0f,     1.0f,
            0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 1.0f,     1.0f,
            1.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,     1.0f, 0.0f,     1.0f,
            1.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f,     1.0f, 1.0f,     1.0f,
        };

        unsigned int indices[] = {
            0, 1, 2,    1, 2, 3,
            4, 5, 6,    5, 6, 7
        };
        vb = new VertexBuffer(vertices, sizeof(vertices), GL_STATIC_DRAW);
        layout = new VertexBufferLayout();
        layout->Push<float>(2);
        layout->Push<float>(4);
        layout->Push<float>(2);
        layout->Push<float>(1);

        ib = new IndexBuffer(indices, 12);

        va = new VertexArray();
        va->AddBuffer(*vb, *layout);

        shader = new Shader("res/shaders/colorShader.shader");

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
        int values[2] = {0, 1};
        shader->SetUniform1iv("u_Textures", 2, values);
    }
    QuadBashTest::~QuadBashTest()
    {
        delete vb;
        delete layout;
        delete ib;
        delete va;
        delete shader;

        delete tex1;
        delete tex2;
    }

    void QuadBashTest::OnRender()
    {
        shader->Bind();
        
        model = glm::translate(glm::mat4(1.0f), translation)
        * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0.0f,0.0f,1.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(scale,scale,1));

        mvp = projection * view * model;


        Renderer::Draw(*va, *ib, *shader);
        shader->SetUniformMat4f("u_MVP", mvp);
    }
    void QuadBashTest::OnImGuiRender()
    {
        ImGui::Begin("Texture/vertex transforming");
        ImGui::SliderFloat2("Position", &translation.x, -2.0, 2.0f);
        ImGui::SliderFloat("Scale", &scale, 0.0f, 2.0f);
        ImGui::SliderFloat("Rotation", &rotation, -3.14f, 3.14f);
        ImGui::End();
    }
}