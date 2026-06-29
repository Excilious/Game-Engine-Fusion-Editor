#include "Player.h"

Player::Player(void) {};

void Player::SetPlayerSpeed(float NewPlayerSpeed)   {this->PlayerSpeed = NewPlayerSpeed;}
float Player::GetPlayerSpeed(void)                  {return this->PlayerSpeed;}

void Player::SpawnPlayer(Parser* EngineParser,const std::string& MeshSource,const glm::vec3& Translation,const glm::quat& Rotation,const glm::vec3& Scale,const std::string& EntitySpawns)
{
    this->SpawnActor(EngineParser,MeshSource,Translation,Rotation,Scale,EntitySpawns);
}

void Player::MovePlayer(GLFWwindow* EngineWindow,const glm::vec3& Orientation)
{
    this->NextPosition = glm::vec3(0.0f,0.0f,0.0f);
    if (glfwGetKey(EngineWindow,GLFW_KEY_W) == GLFW_PRESS) {this->NextPosition += this->PlayerSpeed*Orientation;}
    if (glfwGetKey(EngineWindow,GLFW_KEY_A) == GLFW_PRESS) {this->NextPosition += this->PlayerSpeed*-glm::normalize(glm::cross(Orientation, glm::vec3(0.0f, 1.0f, 0.0f)));}
    if (glfwGetKey(EngineWindow,GLFW_KEY_S) == GLFW_PRESS) {this->NextPosition += this->PlayerSpeed*-Orientation;}
    if (glfwGetKey(EngineWindow,GLFW_KEY_D) == GLFW_PRESS) {this->NextPosition += this->PlayerSpeed*glm::normalize(glm::cross(Orientation, glm::vec3(0.0f, 1.0f, 0.0f)));}
}

void Player::UpdatePlayer(GLFWwindow* EngineWindow,Shader* ShaderInstance,Camera* CameraInstance,float NearPlane,float FarPlane,float FieldOfView,float DeltaTime)
{
    this->MovePlayer(EngineWindow,CameraInstance->Orientation);
    CameraInstance->BindCameraMatrix(
        FieldOfView,NearPlane,FarPlane,
        this->GetModelPosition()+glm::vec3(0.0f,this->GetModelScale().y,0.0f)*
        this->PlayerEyeOffset
    );

    if (std::get<bool>(Commands::GetFromRegister("thirdperson"))) {this->DrawActor(ShaderInstance,CameraInstance,NearPlane,FarPlane,FieldOfView,DeltaTime);}
}