#ifndef __ACTOR_HEADER__
#define __ACTOR_HEADER__

#define PLAYER_SPAWN_OFFSET 50.0f

#include <string>

#include "../Rendering/Models/Model.h"
#include "../Common/Console.h"
#include "../Common/Parser.h"
#include "Animation.h"

class Actor: public Model
{
    public:
                                    Actor(void);
        void                        SpawnActor(Parser* EngineParser,const std::string& MeshSource,const glm::vec3& Translation,const glm::quat& Rotation,const glm::vec3& Scale,const std::string& EntitySpawns);
        void                        DrawActor(Shader* ShaderInstance,Camera* CameraInstance,float NearPlane,float FarPlane,float FieldOfView,float DeltaTime);
        void                        DestroyActor(void);
    private:
        std::vector<glm::vec3>      PlayerSpawnPositions;
        Animation                   NewAnimationSystem;
        AnimationNode               NewAnimation;

        Node                        StaticAnimationNode;
        Node                        AnimatedAnimationNode;
};

#endif