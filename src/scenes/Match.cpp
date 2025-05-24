#include "Match.h"
#include "GameManager.h"

#include "imgui/imgui.h"

namespace scene{
    void Wall::setAtive(bool active)
    {
        b2Filter filter = b2Shape_GetFilter(shapeId);
        if(active){
            filter.maskBits = 0xFFFF;
        }else{
            filter.maskBits = 0x0000;
        }
        b2Shape_SetFilter(shapeId, filter);
    }

    void Ball::loadBody(b2WorldId worldId)
    {
        b2BodyDef ballBodyDef = b2DefaultBodyDef();
        ballBodyDef.type = b2_dynamicBody;
        ballBodyDef.position = (b2Vec2){0.0f, 0.0f};
        ballBodyDef.linearDamping = 1.0f;

        bodyId = b2CreateBody(worldId, &ballBodyDef);

        b2Circle ballCircle;
        ballCircle.center = (b2Vec2){0.0f, 0.0f};
        ballCircle.radius = 0.25f;
        b2ShapeDef ballShapeDef = b2DefaultShapeDef();
        ballShapeDef.density = 1.0f;
        ballShapeDef.friction = 0.0f;
        ballShapeDef.restitution = 1.0f;

        b2CreateCircleShape(bodyId, &ballShapeDef, &ballCircle);
    }

    void Ball::update(float deltaTime)
    {
        if(abs(b2Body_GetLinearVelocity(bodyId).x) <= 0.09f && abs(b2Body_GetLinearVelocity(bodyId).y) <= 0.09f){
            b2Body_SetLinearVelocity(bodyId, {0.0f, 0.0f});
        }
        if(state == ON_AIR){
            air_time += 1.0f * deltaTime;
            height = -pow((2*air_time - 1), 2) + 1;

            if(height <= 0.0f){
                height = 0.0f;
                air_time = 0.0f;
                onGroundContact();
            }
        }
        
    }

    void Ball::setImpulse()
    {
        state = ON_AIR;
        b2Body_SetLinearDamping(bodyId, 0.0f);
    }

    void Ball::onGroundContact()
    {
        state = ON_GROUND;
        b2Body_SetLinearDamping(bodyId, 1.0f);
    }

    Match::Match(GameManager *manager) : Scene(manager)
    {
        camPos = {0.0f, 0.0f, 0.0f, 0.0f};
        camPosOffset = {0.0f, 0.0f, 0.0f, 0.0f};
        
        zoom = 1.0f;
        view = glm::translate(glm::mat4(1.0f), {camPos.x + camPosOffset.x, camPos.y + camPosOffset.y, 0.0f}) * glm::scale(glm::mat4(1.0f),glm::vec3(zoom, zoom, 1.0f));


        // Box2d world setup
        b2WorldDef worldDef;
        worldDef = b2DefaultWorldDef();
        worldDef.gravity = (b2Vec2){0.0f, 0.0f};

        worldId = b2CreateWorld(&worldDef);

        b2BodyDef groundBodyDef = b2DefaultBodyDef();
        groundBodyDef.position = (b2Vec2){-1.8f, -3.0f};
        groundBodyDef.rotation = b2MakeRot(1.0f);

        groundBodyId = b2CreateBody(worldId, &groundBodyDef);

        b2Polygon groundBox = b2MakeBox(1.5f, 1.5f);
        b2ShapeDef groundShapeDef = b2DefaultShapeDef();
        groundShapeDef.isSensor = true;
        b2CreatePolygonShape(groundBodyId, &groundShapeDef, &groundBox); 

        std::cout << "size of b2Vec2: "<<sizeof(b2Vec2) << "; \n";
        std::cout << "size of float: "<<sizeof(float) << "\n";

        walls.push_back({{0}, {-0.25, 1}, {-1.25, 0}, 4, 0.5f, false});
        walls.push_back({{0}, {-0.75, 2.5}, {-0.25, 1}, -2, 0.5f, true});
        walls.push_back({{0}, {2, 2.75}, {-0.75, 2.5}, 11, 0.5f, false});
        walls.push_back({{0}, {2.75, 1}, {2, 0.25}, 3, 0.5f, false});
        walls.push_back({{0}, {2, 0.25}, {3.25, 0}, -5, 0.5f, false});
        walls.push_back({{0}, {2.75, 1}, {3.25, 0}, -2, 0.5f, true});

        for(int i = 0; i < walls.size(); i++){
            b2BodyDef segmentBodyDef = b2DefaultBodyDef();
            walls[i].segmentId = b2CreateBody(worldId, &segmentBodyDef);

            b2ShapeDef segmentShapeDef = b2DefaultShapeDef();
            b2Segment segment = {walls[i].point1, walls[i].point2};

            walls[i].shapeId = b2CreateSegmentShape(walls[i].segmentId, &segmentShapeDef, &segment);
        }

        ball.loadBody(worldId);
        
        //-------------------------
    }

    Match::~Match()
    {
        b2DestroyWorld(worldId);
    }

    void Match::OnUpdate(float deltaTime)
    {
        zoom += GameManager::scrollFactor * deltaTime;
        if(zoom < 0.1f){zoom = 0.1f;}
        if(zoom > 3.0f){zoom = 3.0f;}
        GameManager::scrollFactor = 0;

        b2World_Step(worldId, deltaTime, 4);
        ball.update(deltaTime);
        for(int i = 0; i < walls.size(); i++){
            if(!walls[i].tall && ball.height > 0.1f){
                walls[i].setAtive(false);
            }
            if(!walls[i].tall && ball.height <= 0.1f){
                walls[i].setAtive(true);
            }
        }
    }

    void Match::OnRender()
    {
        view = glm::scale(glm::mat4(1.0f),glm::vec3(zoom, zoom, 1.0f)) 
            * glm::translate(glm::mat4(1.0f), {camPos.x + camPosOffset.x, camPos.y + camPosOffset.y, 0.0f});

        manager->useShader(GM_BASIC_SHADER);
        manager->activeShader->SetUniformMat4f("u_Projection", manager->projection);
        manager->activeShader->SetUniformMat4f("u_View", view);

        Renderer::drawShape(manager->circle, b2Body_GetPosition(ball.bodyId).x, b2Body_GetPosition(ball.bodyId).y, 0.5f,{1.0f, 0.0f, 0.0f, 1.0f});
        float base[2] = {b2Body_GetPosition(ball.bodyId).x, b2Body_GetPosition(ball.bodyId).y};
        float altitude[2] = {b2Body_GetPosition(ball.bodyId).x, b2Body_GetPosition(ball.bodyId).y + ball.height};
        Renderer::drawLine(manager->line, base, altitude, {1.0f, 1.0f, 1.0f, 1.0f});
        
        for(int i = 0; i < walls.size(); i++){

            Renderer::drawLine(manager->line, (float*)&walls[i].point1, (float*)&walls[i].point2, (walls[i].tall) ? glm::vec4{1.0f, 0.0f, 0.0f, 1.0f} : glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});
        }

        Renderer::drawShape(manager->square, b2Body_GetPosition(groundBodyId).x, b2Body_GetPosition(groundBodyId).y, 3.0f, 3.0f, 1.0f, {0.0f, 0.0f, 1.0f, 1.0f});

        Renderer::drawShape(manager->square, mouseWorldPos.x, mouseWorldPos.y, 0.2f, 0.2f, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f});
        
    }



    void Match::OnImGuiRender()
    {
        ImGui::Begin("Camera stats");
        ImGui::Text("OffsetPos: x: %.2f y: %.2f", camPosOffset.x, camPosOffset.y);
        ImGui::Text("Pos: x: %.2f y: %.2f", camPos.x, camPos.y);

        ImGui::Text("MouseNormPos: x: %.2f y: %.2f", manager->mouseNormPos.x, manager->mouseNormPos.y);
        ImGui::Text("ballVelocity: x: %.2f y: %.2f", abs(b2Body_GetLinearVelocity(ball.bodyId).x), abs(b2Body_GetLinearVelocity(ball.bodyId).y));
        ImGui::End();
    }

    void Match::HandleEvents(GLFWwindow *window)
    {
        mouseWorldPos = (manager->mouseNormPos)/zoom - camPos - camPosOffset;

        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
            if(!dragging){
                dragging = true;
                startMouseWorldPos = manager->mouseNormPos;
            }

            camPosOffset = {(manager->mouseNormPos.x - startMouseWorldPos.x)/zoom, (manager->mouseNormPos.y - startMouseWorldPos.y)/zoom, 0.0f, 0.0f};

        }

        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE){
            if(dragging){
                dragging = false;
                camPos += camPosOffset;
                camPosOffset = {0.0f, 0.0f, 0.0f, 0.0f};
            }
        }

        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && ball.state == ON_GROUND){
            ball.setImpulse();
        }

        if(ImGui::GetIO().WantCaptureMouse){return;}
        if(b2Body_GetLinearVelocity(ball.bodyId).x == 0.0f && b2Body_GetLinearVelocity(ball.bodyId).y == 0.0f){
            if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
                std::cout << "added impulse \n";
                b2Vec2 impulse = (b2Vec2){(b2Body_GetPosition(ball.bodyId).x - mouseWorldPos.x) * 25, 
                    (b2Body_GetPosition(ball.bodyId).y - mouseWorldPos.y) * 25};
                b2Body_ApplyForceToCenter(ball.bodyId, impulse, true);
            }
        }
    }
    
}