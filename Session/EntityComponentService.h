#ifndef __ENTITY_COMPONENT_SERVICE__
#define __ENTITY_COMPONENT_SERVICE__

#include <vector>

#include "../Rendering/Models/Model.h"
#include "../Entity/Actor.h"

enum EntityType
{
    ENTITY_ACTOR,
    ENTITY_MODEL
};

struct EntityComponents
{
    Model           ModelGroup;
    Actor           ActorGroup;
    std::string     MeshSource;
    EntityType      ModelType;
};

class EntityComponentService
{
    public:
                                        EntityComponentService(void);
        void                            AddEntity(Parser* EngineParser,const std::string& MeshSource,const glm::vec3& Translation,const glm::quat& Rotation,const glm::vec3& Scale);
        void                            AddActor(Parser* EngineParser,const std::string& MeshSource,const glm::vec3& Translation,const glm::quat& Rotation,const glm::vec3& Scale,const std::string WorldString);
        void                            RemoveEntity(const std::string& MeshSource);
        void                            RemoveActor(const std::string& MeshSource);

        void                            RenderGroup(Shader* EngineShader,Camera* EngineCamera,float NearView,float FarView,float FieldOfView,float DeltaTime);
        void                            DestroyGroup(void);

        Model*                          GetModelInfomation(const std::string MeshSource);
        Actor*                          GetActorInfomation(const std::string MeshSource);
    private:
        std::vector<EntityComponents>   ModelGroups;
};

#endif