#include "EntityComponentService.h"

EntityComponentService::EntityComponentService(void) 
{
    this->ModelGroups = {};
};

Model* EntityComponentService::GetModelInfomation(const std::string MeshSource)
{
    for (int Index = 0; Index < this->ModelGroups.size(); Index++)
    {
        if (this->ModelGroups[Index].MeshSource == MeshSource) 
        {
            return &this->ModelGroups[Index].ModelGroup;
        }
    }

    Console::SysPrint("Failed to get model reference of mesh source '"+MeshSource+"'.",3);
    return nullptr;
}

Actor* EntityComponentService::GetActorInfomation(const std::string MeshSource)
{
    for (int Index = 0; Index < this->ModelGroups.size(); Index++)
    {
        if (this->ModelGroups[Index].MeshSource == MeshSource) 
        {
            return &this->ModelGroups[Index].ActorGroup;
        }
    }

    Console::SysPrint("Failed to get actor reference of mesh source '"+MeshSource+"'.",3);
    return nullptr;
}

void EntityComponentService::AddEntity(Parser* EngineParser,const std::string& MeshSource,const glm::vec3& Translation,const glm::quat& Rotation,const glm::vec3& Scale)
{
    EntityComponents NewComponents;
    Model NewModel;
    
    NewModel.LoadFromGLTF(EngineParser,MeshSource,Translation,Rotation,Scale);
    NewComponents.MeshSource    = MeshSource;
    NewComponents.ModelGroup    = NewModel;   
    NewComponents.ModelType     = ENTITY_MODEL;

    this->ModelGroups.push_back(NewComponents);
}

void EntityComponentService::AddActor(Parser* EngineParser,const std::string& MeshSource,const glm::vec3& Translation,const glm::quat& Rotation,const glm::vec3& Scale,const std::string WorldString)
{
    EntityComponents NewComponents;
    Actor NewActor;
    
    NewActor.SpawnActor(EngineParser,MeshSource,Translation,Rotation,Scale,WorldString);
    NewComponents.MeshSource    = MeshSource;
    NewComponents.ActorGroup    = NewActor;   
    NewComponents.ModelType     = ENTITY_ACTOR;
    
    this->ModelGroups.push_back(NewComponents);
}

void EntityComponentService::RemoveEntity(const std::string& MeshSource)
{
    for (int Index = 0; Index < this->ModelGroups.size(); Index++)
    {
        if (this->ModelGroups[Index].MeshSource == MeshSource) 
        {
            this->ModelGroups[Index].ModelGroup.DestroyModel();
            this->ModelGroups.erase(this->ModelGroups.begin()+Index);
        }
    }
}

void EntityComponentService::RemoveActor(const std::string& MeshSource)
{
    for (int Index = 0; Index < this->ModelGroups.size(); Index++)
    {
        if (this->ModelGroups[Index].MeshSource == MeshSource) 
        {
            this->ModelGroups[Index].ActorGroup.DestroyModel();
            this->ModelGroups.erase(this->ModelGroups.begin()+Index);
        }
    }
}

void EntityComponentService::RenderGroup(Shader* EngineShader,Camera* EngineCamera,float NearView,float FarView,float FieldOfView,float DeltaTime)
{
    for (int Index = 0; Index < this->ModelGroups.size(); Index++)
    {
        if (this->ModelGroups[Index].ModelType == ENTITY_ACTOR)         {this->ModelGroups[Index].ActorGroup.DrawActor(EngineShader,EngineCamera,NearView,FarView,FieldOfView,DeltaTime);}
        else if (this->ModelGroups[Index].ModelType == ENTITY_MODEL)    {this->ModelGroups[Index].ModelGroup.RenderModel(EngineShader,EngineCamera,NearView,FarView,FieldOfView);}
    }
}

void EntityComponentService::DestroyGroup(void)
{
    for (int Index = 0; Index < this->ModelGroups.size(); Index++)
    {
        if (this->ModelGroups[Index].ModelType == ENTITY_ACTOR)         {this->ModelGroups[Index].ActorGroup.DestroyActor();}
        else if (this->ModelGroups[Index].ModelType == ENTITY_MODEL)    {this->ModelGroups[Index].ModelGroup.DestroyModel();}
    }
}