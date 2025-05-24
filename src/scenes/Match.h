#pragma once

#include "Scene.h"
#include "Renderer.h"
#include "box2d/box2d.h"
#include <map>
#include <vector>

namespace scene{
    struct Wall{
        b2BodyId segmentId;
        
        b2Vec2 point1;
        b2Vec2 point2;
        int inclination;
        float tan;
        bool tall;

        b2ShapeId shapeId;

        void setAtive(bool active);
    };

    enum ballStates{
        ON_GROUND = 0,
        ON_AIR = 1,
        ON_PUDDLE = 2
    };

    struct Ball{
        void loadBody(b2WorldId worldId);
        b2BodyId bodyId;

        float height = 0.0f;
        float air_time = 0.0f;
        int state = 0;

        void update(float deltaTime);
        void setImpulse();
        void onGroundContact();
    };

    class Match : public Scene{
    private:
        glm::vec4 camPos;
        float zoom;
        glm::mat4 view;

        b2WorldId worldId;
        b2BodyId groundBodyId;
        Ball ball;

        std::vector<Wall> walls;

        float dragging;
        glm::vec4 startMouseWorldPos;
        glm::vec4 mouseWorldPos;
        glm::vec4 camPosOffset;

    public: 
        Match(GameManager* manager);
        ~Match();

        void OnUpdate(float deltaTime) override;
        void OnRender() override;

        void OnImGuiRender() override;
        void HandleEvents(GLFWwindow* window) override;
    };
}