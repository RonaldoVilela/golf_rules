#include "scenes/DynamicTest.h"
#include "DynamicTest.h"
#include "imgui.h"
#include "GameManager.h"
#include "GLFW/glfw3.h"

namespace scene{
    DynamicTest::DynamicTest(GameManager* manager) : Scene(manager)
    {

        unsigned int indices[] = {
            0, 1, 2,
            1, 2, 3
        };

        struct Vertex{
            float position[2];
            float color[4];
            float coord[2];
        };

        vb = new VertexBuffer(nullptr,sizeof(Vertex) * 100,GL_DYNAMIC_DRAW);

        ib = new IndexBuffer(indices, 6);

        layout = new VertexBufferLayout();
        layout->Push<float>(2);
        layout->Push<float>(4);
        layout->Push<float>(2);

        va = new VertexArray();
        va->AddBuffer(*vb, *layout);

        projection = glm::ortho(-2.0f,2.0f, -1.5f, 1.5f, -1.0f, 1.0f);
        view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));

        translation = glm::vec3(0.0f, 0.0f, 0.0f);
        rotation = 0.0f;
        scale = 1.0f;

        model = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f)), translation);

        mvp = projection * view * model;

        shader = new Shader("res/shaders/shapeShader.shader");
        shader->Bind();
        shader->SetUniformMat4f("u_MVP", mvp);

        shapeShader = new Shader("res/shaders/basicShader.shader");
        
    }

    DynamicTest::~DynamicTest()
    {
        delete vb;
        delete va;
        delete ib;
        delete layout;
        delete shader;

        delete shapeShader;
    }

    void DynamicTest::OnUpdate(float deltaTime)
    {
        //translation.y += 1*deltaTime;

        vb->Bind();
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
    }

    void DynamicTest::OnRender()
    {
        shader->Bind();

        va->Bind();
        shader->Bind();

        model = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f)), translation);
        mvp = projection * view * model;
        shader->SetUniformMat4f("u_MVP", mvp);

        Renderer::Draw(*va, *ib, *shader);

        shapeShader->Bind();
        shapeShader->SetUniformMat4f("u_Projection", projection);
        shapeShader->SetUniformMat4f("u_View", view);
        Renderer::drawShape(manager->circle,0.0f, 0.0f, 0.5f, 0.5f, 0.0f,{0.0f, 1.0f, 0.0f, 1.0f});
    }

    void DynamicTest::OnImGuiRender()
    {
        ImGui::Begin("dynamic testing");
        ImGui::SliderFloat2("vertex1Position",vertices,-1.5f, 1.5f);
        ImGui::End();
    }
}
