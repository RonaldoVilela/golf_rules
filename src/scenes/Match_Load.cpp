#include "Match.h"
#include "GameManager.h"

#include <fstream>
#include <sstream>

namespace scene{

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

            b2BodyDef boundsBodyDef = b2DefaultBodyDef();
            boundsBodyDef.position = (b2Vec2){0.0f, 0.0f};

            mapBoundsId = b2CreateBody(worldId, &boundsBodyDef);

            ball.loadBody(worldId);
            //----------------------------- //


            std::cout << "size of b2Vec2: "<<sizeof(b2Vec2) << "; \n";
            std::cout << "size of float: "<<sizeof(float) << "\n";

            // reading file data here: >>>>>>>>>>

            // bounds >>

            {

            b2ShapeDef boundShapeDef = b2DefaultShapeDef();
            boundShapeDef.isSensor = true;
            int bound_point_count = 0;
            file.read((char*)&bound_point_count, sizeof(int)); // amount of points

            std::cout << "Map bound points: " << bound_point_count << "\n";
            
            for(int i = 0; i < bound_point_count; i++){
                b2Vec2 point;
                
                file.read((char*)&point, sizeof(b2Vec2));
                mapBound_points.push_back(point);
            }

            for(int i = 0; i < bound_point_count; i++){
                b2Segment seg;
                seg.point1 = mapBound_points[i];

                if(i+1 < bound_point_count){
                    seg.point2 = mapBound_points[i+1];
                }else{
                    seg.point2 = mapBound_points[0];
                }
                
                b2CreateSegmentShape(mapBoundsId, &boundShapeDef, &seg);
            }

            //free(bound_points);
            }
            
            std::cout << "now reading the spawn and hole positions: " << "\n";
            // spawn and hole
            file.read((char*)&spawn_position, sizeof(b2Vec2));
            std::cout << "spawn position = " << spawn_position.x << " : " << spawn_position.y << "\n";
            b2Body_SetTransform(ball.bodyId, spawn_position, b2MakeRot(0));

            file.read((char*)&hole_position, sizeof(b2Vec2));

            {
            b2BodyDef holeBodyDef = b2DefaultBodyDef();
            holeBodyDef.type = b2_staticBody;
            holeBodyDef.position = hole_position;

            holeId = b2CreateBody(worldId, &holeBodyDef);

            b2Circle holeCircle;
            holeCircle.center = (b2Vec2){0.0f, 0.0f};
            holeCircle.radius = 0.25f;
            b2ShapeDef holeShapeDef = b2DefaultShapeDef();
            holeShapeDef.isSensor = true;

            b2CreateCircleShape(holeId, &holeShapeDef, &holeCircle);
            }
            
            std::cout << "hole position = " << hole_position.x << " : " << hole_position.y << "\n";

            // groups >>

            int groupCount = 0;
            file.read((char*)&groupCount, sizeof(int)); // amount of wall groups
            std::cout << "Map has " << groupCount << " wall groups\n";

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

                    std::cout << "-- wall " << w << " point 1 >> " << newWall.point1.x << " : " << newWall.point1.x << "\n"; 
                    std::cout << "-- wall " << w << " point 2 >> " << newWall.point2.x << " : " << newWall.point2.x << "\n"; 

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

            course_maps_played.push_back(mapName);
            resources_loaded = true;
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
        mapBound_points.clear();
        groups.clear();
        resources_loaded = false;
    }

    void Match::unload()
    {
        unloadMap();

        actual_course = "";
        course_maps_played.clear();
        GM_LOG("Match resources cleared.");

        resources_loaded = false;
    }
    // ----------------------------------------- //

}