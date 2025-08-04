#include "Match.h"

namespace scene{

    //WALL =======================

    void Wall::setActive(bool active)
    {
        b2Filter filter = b2Shape_GetFilter(shapeId);
        if(active){
            filter.maskBits = 0xFFFF;
        }else{
            filter.maskBits = 0x0000;
        }
        b2Shape_SetFilter(shapeId, filter);
    }
    // ----------------------

    // WALL GROUP ================
    void WallGroup::clearResources()
    {
        //TODO: Implement logic (clears it's texture and other stuff idk)
    }
    // -------------

    // BALL ======================
    void Ball::loadBody(b2WorldId worldId)
    {
        b2BodyDef ballBodyDef = b2DefaultBodyDef();
        ballBodyDef.type = b2_dynamicBody;
        ballBodyDef.position = (b2Vec2){0.0f, 0.0f};
        ballBodyDef.linearDamping = 1.0f;
        ballBodyDef.isBullet = true;

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
        b2Body_SetLinearDamping(bodyId, 0.35f);
    }

    void Ball::onGroundContact()
    {
        state = ON_GROUND;
        b2Body_SetLinearDamping(bodyId, 1.0f);
    }
    // -------------------------------

    
}