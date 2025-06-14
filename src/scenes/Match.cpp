#include "Match.h"
#include "GameManager.h"

#include "imgui/imgui.h"
#include <fstream>

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

        //WARNING: This is a temporary code, a map and couse should NEVER be loaded in the Match class constructor;
        //loadCourse("db_testcourse");
        //loadMap("Untitled");
        
    }

    Match::~Match()
    {
        b2DestroyWorld(worldId);
    }

    // LOAD AND UNLOAD FILES ========================= //

    int Match::loadCourse(std::string courseName)
    {
        actual_course = courseName;
        return 0;
    }

    int Match::loadMap(std::string mapName)
    {
        if(actual_course.length() == 0){
            GM_LOG("Error loading Map: Can't load a map without loading it's course folder first! >> returns: 1",LOG_ERROR);
            return 1;
        }

        std::ifstream file("res/maps/"+ actual_course+"/"+mapName+".grm", std::ios::binary);

        if(file.is_open()){
            // Box2d world setup --------- //
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

            ball.loadBody(worldId);
            //----------------------------- //


            std::cout << "size of b2Vec2: "<<sizeof(b2Vec2) << "; \n";
            std::cout << "size of float: "<<sizeof(float) << "\n";

            // reading file data here: >>>>>>>>>>

            int groupCount = 0;
            file.read((char*)&groupCount, sizeof(int));

            for(int i = 0; i < groupCount; i++){
                
                groups.push_back(WallGroup());
                std::cout << "Creating group: "<< i <<"\n";
                int wallCount = 0;
                file.read((char*)&wallCount, sizeof(int));
                 std::cout << "Group "<< i << " have " << wallCount << " walls .\n";

                for(int w = 0; w < wallCount; w++){
                    std::cout << "Creating wall "<< w << " of the group: "<< i <<"\n";
                    //read the segment data
                    //order: pont1 | point2 | inclination | tan | is_tall
                    Wall newWall;
                    file.read((char*)&newWall.point1, sizeof(b2Vec2));
                    file.read((char*)&newWall.point2, sizeof(b2Vec2));

                    file.read((char*)&newWall.inclination, sizeof(int));
                    file.read((char*)&newWall.tan, sizeof(float));
                    file.read((char*)&newWall.tall, sizeof(bool));

                    groups[i].walls.push_back(newWall);

                    // Create box2d body
                    b2BodyDef segmentBodyDef = b2DefaultBodyDef();
                    groups[i].walls[w].segmentId = b2CreateBody(worldId, &segmentBodyDef);

                    b2ShapeDef segmentShapeDef = b2DefaultShapeDef();
                    b2Segment segment = {groups[i].walls[w].point1, groups[i].walls[w].point2};

                    groups[i].walls[w].shapeId = b2CreateSegmentShape(groups[i].walls[w].segmentId, &segmentShapeDef, &segment);
                }
                std::cout << "Created all walls from group: "<< i <<"\n";
            }
            file.close();
            return 0;
        }else{
            GM_LOG("Error loading Map: Couldn't find the map file: ["+ mapName +".grm] in the folder of the course ["+actual_course+"] >> returns: 1",LOG_ERROR);
            return 1;
        }
    }

    void Match::unloadMap()
    {
        b2DestroyWorld(worldId);
        for(WallGroup g: groups){
            g.clearResources();
        }
        groups.clear();
    }

    void Match::unload()
    {
        unloadMap();

        actual_course = "";
        course_maps_played.clear();
        GM_LOG("Match resources cleared.");
    }
    // ----------------------------------------- //

    void Match::OnUpdate(float deltaTime)
    {
        zoom += GameManager::scrollFactor * deltaTime;
        if(zoom < 0.1f){zoom = 0.1f;}
        if(zoom > 3.0f){zoom = 3.0f;}
        GameManager::scrollFactor = 0;

        b2World_Step(worldId, deltaTime, 4);
        ball.update(deltaTime);

        //  Check if the wall should be active, acoording to the ball height.
        for(WallGroup g: groups){
            for(int i = 0; i < g.walls.size(); i++){
                if(!g.walls[i].tall && ball.height > 0.1f){
                    g.walls[i].setAtive(false);
                }
                if(!g.walls[i].tall && ball.height <= 0.1f){
                    g.walls[i].setAtive(true);
                }
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
        
        for(WallGroup g: groups){
            for(int i = 0; i < g.walls.size(); i++){
                Renderer::drawLine(manager->line, (float*)&g.walls[i].point1, (float*)&g.walls[i].point2, (g.walls[i].tall) ? glm::vec4{1.0f, 0.0f, 0.0f, 1.0f} : glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});
            }
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

    //TODO: Move this function to a different location, maybe create a .cpp file just for struct function declaraions.
    void WallGroup::clearResources()
    {
    }

}