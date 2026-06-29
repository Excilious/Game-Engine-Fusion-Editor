#include "Camera.h"

Camera::Camera(void) 
{
	this->Position  = glm::vec3(0.0f, 0.0f, 0.0f);
    this->Width     = 0;
	this->Height    = 0;
    this->Near      = 0;
    this->Far       = 0;
}

glm::vec3& Camera::ReturnCameraPosition(void)       {return this->Position;}
glm::mat4& Camera::ReturnProjectionMatrix(void)     {return this->ProjectionMatrix;}
glm::mat4& Camera::ReturnViewMatrix(void)           {return this->ViewMatrix;}

void Camera::UpdateCameraPosition(const glm::vec3& NewPosition)         {this->Position = NewPosition;}
void Camera::UpdateCameraResolution(int WindowWidth,int WindowHeight)   {this->Width = WindowWidth; this->Height = WindowHeight;}

void Camera::SetViewportSize(float Width, float Height,float FieldOfView,float Near,float Far)
{
    if (Height <= 0.0f) {Height = 1.0f;}

    float AspectRatio   = Width / Height;
    this->Width         = Width;
    this->Height        = Height;

    this->ProjectionMatrix = glm::perspective(glm::radians(FieldOfView),AspectRatio,Near,Far);
}

void Camera::SpawnCamera(int WindowWidth,int WindowHeight,const glm::vec3& CameraPosition)
{
    this->Width         = WindowWidth;
    this->Height        = WindowHeight;
    this->Position      = CameraPosition;
}

void Camera::UpdateCameraMatrix(float FieldOfView,float NearPlane,float FarPlane)
{
    this->Near              = NearPlane;
    this->Far               = FarPlane;
    this->ProjectionMatrix  = glm::perspective(glm::radians(FieldOfView),(float)(this->Width/this->Height), NearPlane,FarPlane);
    this->ViewMatrix        = glm::lookAt(this->Position,this->Position+this->Orientation,this->UpPosition);
}

void Camera::BindCameraMatrix(float FieldOfView,float NearPlane,float FarPlane,const glm::vec3& BindedPosition)
{
    this->Near              = NearPlane;
    this->Far               = FarPlane;
    this->ProjectionMatrix  = glm::perspective(glm::radians(FieldOfView),(float)(this->Width/this->Height),NearPlane,FarPlane);
    this->ViewMatrix        = glm::lookAt(BindedPosition,BindedPosition+this->Orientation,this->UpPosition);
}

void Camera::MoveCamera(void)
{
    if (Console::IsConsoleOpen) {return;}

    float RotationX;
    float RotationY;
    double MouseX = 0.0;
    double MouseY = 0.0;
    RotationX = this->Sensitivity * (float)(MouseY - (this->Height/2)) / this->Height;
    RotationY = this->Sensitivity * (float)(MouseX - (this->Width/2)) / this->Width;

    glm::vec3 NewOrientation = glm::rotate(this->Orientation,glm::radians(-RotationX),glm::normalize(glm::cross(this->Orientation,this->UpPosition)));

    if (!((glm::angle(NewOrientation,UpPosition) <= glm::radians(5.0f)) || (glm::angle(NewOrientation,-UpPosition) <= glm::radians(5.0f))))
    {this->Orientation  = NewOrientation;}
    this->Orientation   = glm::rotate(this->Orientation,glm::radians(-RotationY),this->UpPosition);
}

void Camera::UpdateCameraView(GLFWwindow* EngineWindow,double XPosition,double YPosition)
{
    if (Console::IsConsoleOpen) {this->FirstMouse = true; return;}
    if (this->FirstMouse)
    {
        this->LastX         = XPosition;
        this->LastY         = YPosition;
        this->FirstMouse    = false;
    }

    this->XOffset   = (float)XPosition - this->LastX;
    this->YOffset   = (float)this->LastY - YPosition;
    this->LastX     = XPosition;
    this->LastY     = YPosition;

    this->XOffset *= this->Sensitivity;
    this->YOffset *= this->Sensitivity;
    this->RotateX += this->XOffset;
    this->RotateY += this->YOffset;

    if (this->RotateY > 89.0f)  {this->RotateY = 89.0f;}
    if (this->RotateY < -89.0f) {this->RotateY = -89.0f;}

    this->Orientation.x = cos(glm::radians(this->RotateY)) * cos(glm::radians(this->RotateX));
    this->Orientation.y = sin(glm::radians(this->RotateY));
    this->Orientation.z = cos(glm::radians(this->RotateY)) * sin(glm::radians(this->RotateX));
    this->Orientation   = glm::normalize(this->Orientation);
}

void Camera::MoveFunctionCamera(GLFWwindow* EngineWindow)
{
    if (glfwGetKey(EngineWindow, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)
    {
        if (glfwGetKey(EngineWindow, GLFW_KEY_W) == GLFW_PRESS) { this->Position += 5.0f * Orientation; }
        if (glfwGetKey(EngineWindow, GLFW_KEY_A) == GLFW_PRESS) { this->Position += 5.0f * -glm::normalize(glm::cross(Orientation, glm::vec3(0.0f, 1.0f, 0.0f))); }
        if (glfwGetKey(EngineWindow, GLFW_KEY_S) == GLFW_PRESS) { this->Position += 5.0f * -Orientation; }
        if (glfwGetKey(EngineWindow, GLFW_KEY_D) == GLFW_PRESS) { this->Position += 5.0f * glm::normalize(glm::cross(Orientation, glm::vec3(0.0f, 1.0f, 0.0f))); }
    }
}