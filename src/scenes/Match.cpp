#include "Match.h"
#include "GameManager.h"

#include "imgui/imgui.h"
#include <fstream>

namespace scene{
    
    void Match::manageMatchEvents()
    {
        while(manager->eventList.size() > 0){

            
            std::array<char, 256> temp = manager->eventList.front();

            GameEvent event(temp.data());

            if(manager->player.onlineStatus == SV_CLIENT){
                switch(event.getType()){

                    case game_event::SERVER_CLOSED:
                            manager->disconnect();
                        break;

                    case game_event::PLAYER_CONNECTION: 
                        {

                            Player newPlayer;

                            int name_size = event.readInt();
                            char *name = (char*)malloc(name_size + 1);
                            name[name_size] = '\0';

                            event.readData(name,name_size);

                            strcpy(newPlayer.m_name, name);

                            newPlayer.connection_id = event.readInt();
                            manager->connected_players.insert(std::make_pair(newPlayer.connection_id, newPlayer));

                            GM_LOG(std::string(newPlayer.m_name) + " joined the crew!");
                        }
                        break;
                    
                    case game_event::SERVER_INFO_UPDATE:
                        GameManager::actual_server.setServerInfo(event.readData(256-sizeof(int)));
                        break;

                    case game_event::PLAYER_DISCONNECTION:
                        {
                            int disconnected_player_id = event.readInt();
                            GM_LOG(std::string(manager->connected_players.at(disconnected_player_id).m_name) + " left the room");
                            manager->connected_players.erase(disconnected_player_id);
                        }
                        break;
                    case game_event::MATCH_FORCED_TERMINATION:
                        {
                            manager->changeScene("lobby");
                        }
                        break;

                }
            }
            else{ // if is hosting

                switch(event.getType()){
                    case game_event::MATCH_MAP_REQUEST:
                    
                        std::cout << "received the map request\n";
                        int player_conn_id = event.readInt();

                        GameEvent responseEvent(game_event::MATCH_MAP_RESPONSE);

                        // course name

                        responseEvent.pushData((int)actual_course.size());
                        responseEvent.pushData(actual_course.data(),(int)actual_course.size());

                        // hole name
                        responseEvent.pushData((int)course_maps_played.back().size());
                        responseEvent.pushData(course_maps_played.back().data(), (int)course_maps_played.back().size());

                        manager->sendEventTo(responseEvent.getData(), manager->connected_players[player_conn_id].m_socket);
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

    }

    Match::~Match()
    {
        if(resources_loaded){
            b2DestroyWorld(worldId);
        }
    }

    void Match::start()
    {
        player_role = SPECTATOR;
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
        if(!resources_loaded){return;}

        // camera ----
        zoom += GameManager::scrollFactor * deltaTime;
        if(zoom < 0.1f){zoom = 0.1f;}
        if(zoom > 3.0f){zoom = 3.0f;}
        GameManager::scrollFactor = 0;

        b2World_Step(worldId, deltaTime, 4);
        ball.update(deltaTime);
        //------------

        //  Check map bounds
        {
        b2DistanceInput input;

        input.transformA = b2Body_GetTransform(ball.bodyId);
        b2Vec2 ballCenter = {0.0f, 0.0f};
        
        input.proxyA = b2MakeProxy(&ballCenter, 1, 0.0f);

        input.transformB = b2Body_GetTransform(mapBoundsId);
        b2SimplexCache cache = {0};
        b2Simplex simplexBuffer[1];

        int lineCount = b2Body_GetShapeCount(mapBoundsId);
        float shorterDistance;

        for(int i = 0; i < lineCount; i++){
            
            b2Vec2 verts[2];
            if(i + 1 >= lineCount){
                verts[0] = mapBound_points[i];
                verts[1] = mapBound_points[0];
            }else{
                verts[0] = mapBound_points[i];
                verts[1] = mapBound_points[i+1];
            }
            input.proxyB = b2MakeProxy(verts, 2, 0);

            b2DistanceOutput output = b2ShapeDistance(&cache, &input, simplexBuffer, 1);

            b2Vec2 ball_point = output.pointA;
            if(i == 0){shorterDistance = b2Distance(output.pointA, output.pointB);}
            
            if(b2Distance(output.pointA, output.pointB) <= shorterDistance){
                shorterDistance = b2Distance(output.pointA, output.pointB);
                b2Vec2 bound_point = output.pointB;

                b2Vec2 dir = {verts[0].x - verts[1].x, verts[0].y - verts[1].y};
                b2Vec2 norm = {-dir.y, dir.x};

                b2Vec2 distanceVec = {ball_point.x - bound_point.x, ball_point.y - bound_point.y};
                if(distanceVec.x*norm.x + distanceVec.y*norm.y < 0){
                    ball.out_of_bounds = true;
                }else{
                    ball.out_of_bounds = false;
                }
            }
            
        }

        } // end of temporary scope
        

        //  Check if the wall should be active, acoording to the ball height.
        for(WallGroup g: groups){
            for(int i = 0; i < g.walls.size(); i++){
                if(!g.walls[i].tall && ball.height > 0.1f){
                    g.walls[i].setActive(false);
                }
                if(!g.walls[i].tall && ball.height <= 0.1f){
                    g.walls[i].setActive(true);
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

        Renderer::drawSegment(manager->line, (float*)&mapBound_points[0], mapBound_points.size(),{0.0f, 0.0f, 1.0f, 1.0f});

        Renderer::drawShape(manager->circle, b2Body_GetPosition(holeId).x, b2Body_GetPosition(holeId).y, 0.5f,{0.0f, 1.0f, 0.5f, 1.0f});

        Renderer::drawShape(manager->circle, b2Body_GetPosition(ball.bodyId).x, b2Body_GetPosition(ball.bodyId).y, 0.5f,{1.0f, 0.0f, 0.0f, 1.0f});
        float base[2] = {b2Body_GetPosition(ball.bodyId).x, b2Body_GetPosition(ball.bodyId).y};
        float altitude[2] = {b2Body_GetPosition(ball.bodyId).x, b2Body_GetPosition(ball.bodyId).y + ball.height};
        Renderer::drawLine(manager->line, base, altitude, {1.0f, 1.0f, 1.0f, 1.0f});
        
        for(WallGroup g: groups){
            for(int i = 0; i < g.walls.size(); i++){
                Renderer::drawLine(manager->line, (float*)&g.walls[i].point1, (float*)&g.walls[i].point2, (g.walls[i].tall) ? glm::vec4{1.0f, 0.0f, 0.0f, 1.0f} : glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});
            }
        }

        //Renderer::drawShape(manager->square, b2Body_GetPosition(gId).x, b2Body_GetPosition(groundBodyId).y, 3.0f, 3.0f, 1.0f, {0.0f, 0.0f, 1.0f, 1.0f});

        Renderer::drawShape(manager->square, mouseWorldPos.x, mouseWorldPos.y, 0.2f, 0.2f, 0.0f, {1.0f, 1.0f, 1.0f, 1.0f});

        manager->ui_FrameBuffer.Bind();

        GLCall(glActiveTexture(GL_TEXTURE0));
        GLCall(glBindTexture(GL_TEXTURE_2D, manager->font.textureID));

        GameManager::useShader(GM_FONT_SHADER);
        GameManager::activeShader->SetUniformMat4f("u_MVP", manager->windowProjection);

        switch(player_role){
            case SPECTATOR: Renderer::drawString("[Spectating...]", 0, 20,0.8f, {1.0f, 1.0f, 1.0f, 1.0f});
                break;
            case PLAYER_1: Renderer::drawString("[Player 1]", 0, 20,0.8f, {0.0f, 0.0f, 1.0f, 1.0f});
                break;
            case PLAYER_2: Renderer::drawString("[Player 2]", 0, 20,0.8f, {1.0f, 0.0f, 0.0f, 1.0f});
                break;
        }

        if(ball.out_of_bounds){
            Renderer::drawString("Ball out", 0, 80,0.8f, {1.0f, 1.0f, 0.0f, 1.0f});
        }else{
            Renderer::drawString("Ball in", 0, 80,0.8f, {1.0f, 1.0f, 1.0f, 1.0f});
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

            camPosOffset = {(manager->mouseNormPos.x - startMouseWorldPos.x)/zoom, // x
                            (manager->mouseNormPos.y - startMouseWorldPos.y)/zoom, // y
                            0.0f, 0.0f};                                           // z

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
                //std::cout << "added impulse \n";
                b2Vec2 direction = (b2Vec2){(b2Body_GetPosition(ball.bodyId).x - mouseWorldPos.x), 
                    (b2Body_GetPosition(ball.bodyId).y - mouseWorldPos.y)};
                
                float force = sqrt((direction.x*direction.x) + (direction.y*direction.y));

                if(force > 0){
                    direction.x /= force;
                    direction.y /= force;

                    force *= 25;

                    if(force > 55){force = 55;}
                    b2Vec2 impulse = {direction.x * force, direction.y * force};
                    b2Body_ApplyForceToCenter(ball.bodyId, impulse, true);
                }

                
            }
        }
    }

    

}