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
        if(!resources_loaded){return;}

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
            case ESPECTATOR: Renderer::drawString("[Espectating...]", 0, 20,0.8f, {1.0f, 1.0f, 1.0f, 1.0f});
                break;
            case PLAYER_1: Renderer::drawString("[Player 1]", 0, 20,0.8f, {0.0f, 0.0f, 1.0f, 1.0f});
                break;
            case PLAYER_2: Renderer::drawString("[Player 2]", 0, 20,0.8f, {1.0f, 0.0f, 0.0f, 1.0f});
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