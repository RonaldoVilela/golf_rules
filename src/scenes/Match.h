#pragma once

#include "Scene.h"
#include "Renderer.h"
#include "box2d/box2d.h"
#include "box2d/base.h"
#include "box2d/collision.h"
#include "box2d/types.h"
#include "box2d/id.h"
#include "box2d/math_functions.h"
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

        void setActive(bool active);
    };

    struct WallGroup{
        unsigned int texture_id;
        std::vector<Wall> walls;
        float top, bottom;
        //IndexBuffer ib; //triangles
        WallGroup(){};
        void clearResources();
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
        bool out_of_bounds = false;

        void update(float deltaTime);
        void setImpulse();
        void onGroundContact();
    };

    enum MatchRoles{
        SPECTATOR = 0,
        PLAYER_1 = 1,
        PLAYER_2 = 2,
    };

    class Match : public Scene{
    private:
        glm::vec4 camPos;
        float zoom;
        glm::mat4 view;

        b2WorldId worldId;
        b2BodyId mapBoundsId;
        std::vector<b2Vec2> mapBound_points;

        b2BodyId holeId;
        Ball ball;
        
        b2Vec2 spawn_position, hole_position;

        std::vector<WallGroup> groups;

        float dragging;
        glm::vec4 startMouseWorldPos;
        glm::vec4 mouseWorldPos;
        glm::vec4 camPosOffset;

        std::string actual_course = "";
        std::vector<std::string> course_maps_played;

        void manageMatchEvents();
        int player_role = 0;
        int actual_turn = 0;

        bool resources_loaded = false;

    public: 
        Match(GameManager* manager);
        ~Match();

        /**
         * Defines and loads a match course, if a course
         * has been loaded before, it must be unloaded before
         * calling this function.
         * 
         * @param courseName the course name, wich will be the name of the folder
         * where it's information and maps will be.
         * 
         * @return - 0 if it was sucessfully loaded.
         * @return - 1 if the couse folder was not found.
         * @return - 2 if the course couldn't be loaded.
         **/
        int loadCourse(std::string courseName);

        /**
         * Loads a map from the actual course's folder and stores it
         * in a buffer of already played maps.
         * 
         * @attention This function by default loads a random map different from
         * the maps already played before, passing a map name as the paramether
         * loads that specific map, and doesn't add it to the "played maps buffer".
         * 
         * @param mapName name of a specific map from the course.
         * 
         * @return - 0 if it was sucessfully loaded.
         * @return - 1 if the map file was not found.
         * @return - 2 if an error occored while loading the file.
         */
        int loadMap(std::string mapName = nullptr);

        void unloadMap();
        void unload() override;

        void start() override;
        void OnUpdate(float deltaTime) override;
        void OnRender() override;

        void OnImGuiRender() override;
        void HandleEvents(GLFWwindow* window) override;
    };
}