#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameManager;
struct GLFWwindow;

namespace scene{

    class Scene{
    protected:
        GameManager* manager;
    public:
        Scene(GameManager* manager) : manager(manager){}
        virtual ~Scene() {}

        virtual void start() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnRender() {}
        virtual void HandleEvents(GLFWwindow* window) {}

        virtual void OnImGuiRender() {}

    };
}