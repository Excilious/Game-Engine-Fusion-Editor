#include "Actor.h"

Actor::Actor(void) {}

void Actor::SpawnActor(Parser* EngineParser,const std::string& MeshSource,const glm::vec3& Translation,const glm::quat& Rotation,const glm::vec3& Scale,const std::string& EntitySpawns)
{
    srand(time(NULL));
    Console::SysPrint("Loading actor '"+MeshSource+"'...",1);

    this->SetModelPosition(glm::vec3(0.0f,0.0f,0.0f));
}

void Actor::DrawActor(Shader* ShaderInstance,Camera* CameraInstance,float NearPlane,float FarPlane,float FieldOfView,float DeltaTime)
{
    this->NewAnimationSystem.ProduceMatrixModel(this->AnimatedAnimationNode);
    this->RenderModel(ShaderInstance,CameraInstance,NearPlane,FarPlane,FieldOfView);
}

void Actor::DestroyActor(void)
{
    this->DestroyModel();
}