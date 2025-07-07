#include "Match.h"
#include "GameManager.h"

#include "imgui/imgui.h"
#include <fstream>
#include <sstream>

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

    void Match::manageMatchEvents()
    {
        while(manager->eventList.size() > 0){

            
            std::array<char, 256> temp = manager->eventList.front();
            char eventBuffer[256];
            std::memcpy(eventBuffer, temp.data(), 256);

            AbstractEvent* event = (AbstractEvent*)eventBuffer;
            int eventType = event->eventType;

            if(manager->player.onlineStatus == SV_CLIENT){
                switch(eventType){
                    case SERVER_CLOSED_EVENT:
                            manager->disconnect();
                        break;
                    case PLAYER_CONNECTION_EVENT: 
                        {
                            Player newPlayer;
                            strcpy(newPlayer.m_name, event->m_string);
                            newPlayer.connection_id = event->m_flag;
                            manager->connected_players.insert(std::make_pair(newPlayer.connection_id, newPlayer));
                            GM_LOG(std::string(newPlayer.m_name) + " joined the crew!");
                        }
                        break;
                    
                    case SERVER_INFO_UPDATE_EVENT:
                        GameManager::actual_server.setServerInfo(&eventBuffer[sizeof(int)]);
                        break;

                    case PLAYER_DISCONNECTION_EVENT:
                        GM_LOG(std::string(manager->connected_players.at(event->m_flag).m_name) + " left the room");
                        manager->connected_players.erase(event->m_flag);
                        break;

                }
            }else{
                switch(eventType){
                    case MATCH_MAP_REQUEST:
                        std::cout << "received the map request\n";
                        int player_conn_id = eventBuffer[sizeof(int)];
                        std::ostringstream data_buffer(std::ios::binary);
                        int map_index = course_maps_played.size() - 1;

                        int type = MATCH_MAP_RESPONSE;
                        data_buffer.write((char*)&type,sizeof(int));

                        // course name
                        int size = actual_course.size();
                        data_buffer.write((char*)&size,sizeof(int));
                        data_buffer.write(actual_course.c_str(),size);

                        // hole name
                        size = course_maps_played[map_index].size();
                        data_buffer.write((char*)&size,sizeof(int));
                        data_buffer.write(course_maps_played[map_index].c_str(),size);

                        //send(manager->connected_players[player_conn_id].m_socket, data_buffer.str().data(), 256, 0); // directly send the names
                        manager->sendEventTo(data_buffer.str().data(), manager->connected_players[player_conn_id].m_socket);
                        break;
                }
            }
            manager->eventList.pop();
            std::cout << "Received one event here\n";
        };
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

    void Match::start()
    {
        player_role = ESPECTATOR;
        if(manager->player.connection_id == GameManager::actual_server.playerId_1){
            player_role = PLAYER_1;
        }else if(manager->player.connection_id == GameManager::actual_server.playerId_2){
            player_role = PLAYER_2;
        }
    }

    void Match::OnUpdate(float deltaTime)
    {
        if(manager->player.onlineStatus != SV_DISCONNECTED){
            manageMatchEvents();
        }

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

        manager->ui_FrameBuffer.Bind();

        GLCall(glActiveTexture(GL_TEXTURE0));
        GLCall(glBindTexture(GL_TEXTURE_2D, manager->font.textureID));

        GameManager::useShader(GM_FONT_SHADER);
        GameManager::activeShader->SetUniformMat4f("u_MVP", manager->windowProjection);

        switch(player_role){
            case ESPECTATOR: Renderer::drawString("[Espectating...]", 0, 20,0.6f, {1.0f, 1.0f, 1.0f, 1.0f});
                break;
            case PLAYER_1: Renderer::drawString("[Player 1]", 0, 20,0.6f, {0.0f, 0.0f, 1.0f, 1.0f});
                break;
            case PLAYER_2: Renderer::drawString("[Player 2]", 0, 20,0.6f, {1.0f, 0.0f, 0.0f, 1.0f});
                break;
        }
        
        manager->ui_FrameBuffer.Unbind();
        
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