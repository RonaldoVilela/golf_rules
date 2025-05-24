#pragma once

#include "Scene.h"
#include "Renderer.h"

namespace scene{
    class ClearColorTest: public Scene{
    public:
        ClearColorTest(GameManager* manager);
        ~ClearColorTest();
        void OnUpdate(float deltaTime) override;
        void OnRender() override;

        void OnImGuiRender() override;
    
    private:
        float m_ClearColor[4];
    };
}