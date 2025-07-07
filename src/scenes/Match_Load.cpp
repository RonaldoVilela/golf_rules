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
            file.read((char*)&groupCount, sizeof(int)); // amount of wall groups

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

            course_maps_played.push_back(mapName);
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

}