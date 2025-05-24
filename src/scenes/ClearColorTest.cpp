#include "ClearColorTest.h"
#include "imgui/imgui.h"
#include "GameManager.h"
#include "GLFW/glfw3.h"

namespace scene{

    scene::ClearColorTest::ClearColorTest(GameManager* manager) : Scene(manager),
        m_ClearColor {0.2f, 0.3f, 0.8f, 1.0f}
    {
    }

    scene::ClearColorTest::~ClearColorTest()
    {
    }

    void scene::ClearColorTest::OnUpdate(float deltaTime)
    {
    }

    void scene::ClearColorTest::OnRender()
    {
        glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]);
        Renderer::Clear();
    }

    void scene::ClearColorTest::OnImGuiRender()
    {
        ImGui::Begin("Clear Color Picker");
        ImGui::ColorEdit4("Clear color",m_ClearColor);
        ImGui::End();
    }
}
