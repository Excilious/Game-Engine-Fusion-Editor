#ifndef __CAMERA_HEADER__
#define __CAMERA_HEADER__
#define GLM_ENABLE_EXPERIMENTAL

#include "../../Resources/glad/glad.h"
#include "../../Resources/GLFW/glfw3.h"
#include "../../Resources/glm/glm.hpp"
#include "../../Resources/glm/gtc/matrix_transform.hpp"
#include "../../Resources/glm/gtx/rotate_vector.hpp"
#include "../../Resources/glm/gtx/vector_angle.hpp"
#include "../Common/Shader.h"
#include "../Common/Console.h"

class Camera
{
    public:
                        Camera(void);
        void            MoveCamera(void);
        void            SpawnCamera(int WindowWidth,int WindowHeight,const glm::vec3& CameraPosition);
        void            UpdateCameraPosition(const glm::vec3& NewPosition);
        void            UpdateCameraMatrix(float FieldOfView,float NearPlane,float FarPlane);
        void            UpdateCameraResolution(int WindowWidth,int WindowHeight);
        void            UpdateCameraView(GLFWwindow* EngineWindow,double XPosition,double YPosition);
        void            BindCameraMatrix(float FieldOfView,float NearPlane,float FarPlane,const glm::vec3& BindedPosition);
        void            MoveFunctionCamera(GLFWwindow* EngineWindow);

        void            SetViewportSize(float width, float height, float FieldOfView, float Near, float Far);
        glm::vec3&      ReturnCameraPosition(void);
        glm::mat4&      ReturnViewMatrix(void);
        glm::mat4&      ReturnProjectionMatrix(void);

        glm::vec3       UpPosition          = glm::vec3(0.0f,1.0f,0.0f);
        glm::vec3       Orientation         = glm::vec3(0.0f,0.0f,-1.0f);
        int             Width;
        int             Height;
        float             Near;
        float             Far;
    private:

        glm::vec3       Position;
        glm::mat4       ViewMatrix          = glm::mat4(1.0f);
        glm::mat4       ProjectionMatrix    = glm::mat4(1.0f);
        

        float           LastX               = 0;
        float           LastY               = 0;
        float           RotateX             = -90.0f;
        float           RotateY             = 0;
        float           XOffset             = 0;
        float           YOffset             = 0;
        float           Sensitivity         = 0.1f;
        bool            FirstMouse          = true;
};

#endif