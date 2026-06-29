#ifndef __SKYBOX_HEADER__
#define __SKYBOX_HEADER__
#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <string>
#include <vector>

#include "../../Resources/glm/glm.hpp"
#include "../../Resources/glm/gtc/matrix_transform.hpp"
#include "../../Resources/glm/gtc/type_ptr.hpp"
#include "../../Resources/glm/gtx/rotate_vector.hpp"
#include "../../Resources/glm/gtx/vector_angle.hpp"

#include "../../Resources/glad/glad.h"
#include "../../Resources/GLFW/glfw3.h"
#include "../Common/Parser.h"
#include "../Common/Console.h"
#include "../Rendering/Camera.h"

class Skybox
{
    public:
                                        Skybox(void);
        void                            SpawnSkybox(int WindowWidth,int WindowHeight,float SkyNear,float SkyFar,Parser* EngineParser,float FieldOfView);
        void                            RenderSkybox(Camera* EngineCamera,Shader* SkyboxShader,float FieldOfView);
        void                            DestroySkybox(void);
        
        void                            UpdateSkyboxBuffers(Parser* EngineParser, std::vector<std::string> SkyboxShapes);
    private:
        void                            GenerateSkyboxBuffers(Parser* EngineParser, std::vector<std::string> SkyboxShapes);
        unsigned int                    SkyboxVertexArrayObject;
        unsigned int                    SkyboxVertexBufferObject;
        unsigned int                    SkyboxVertexElementObject;
        unsigned int                    CubemapTexture;

        std::vector<ImageFormat>        ImageBuffers;
        int                             Width;
        int                             Height;
        float                           Near;
        float                           Far;
        float                           FieldOfView;
};

#endif