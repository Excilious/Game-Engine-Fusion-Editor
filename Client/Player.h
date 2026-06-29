#ifndef __PLAYER_HEADER__
#define __PLAYER_HEADER__

#include "../Entity/Actor.h"
#include "../Common/Console.h"

class Player: public Actor
{
    public:
                        Player(void);

        void            SpawnPlayer(Parser* EngineParser,const std::string& MeshSource,const glm::vec3& Translation,const glm::quat& Rotation,const glm::vec3& Scale,const std::string& EntitySpawns);
        void            UpdatePlayer(GLFWwindow* EngineWindow,Shader* ShaderInstance,Camera* CameraInstance,float NearPlane,float FarPlane,float FieldOfView,float DeltaTime);
        void            SetPlayerSpeed(float NewPlayerSpeed);
        float           GetPlayerSpeed(void);
    private:
        void            MovePlayer(GLFWwindow* EngineWindow,const glm::vec3& Orientation);
        float           PlayerSpeed;

        glm::vec3       PlayerEyeOffset = glm::vec3(0.0f,24.0f,0.0f);
};

#endif