#ifndef __LIGHTING_HEADER__
#define __LIGHTING_HEADER__

#include <string>
#include <vector>

#include "../../Resources/glm/glm.hpp"
#include "../../Resources/glad/glad.h"
#include "../Common/Shader.h"
#include "Models/Model.h"

struct LightInfomation
{
    Model*      LightModel;
    glm::vec3   LightPosition;
    glm::vec3   LightDirection;
    glm::vec3   LightColour;
    
    float       LightAmbience;
    float       LightSpecular;
    float       LightSpecularStrength;
    
    float       LightRadius;
    float       LightLinear;
    float       LightQuadratic;

    double      LightCutoff;
    double      LightOuterCutoff;
    float       LightBrightnes;
};

class Lighting
{
    public:
                                        Lighting(void);
        void                            UploadLightingToScene(LightInfomation CurrentLightInfomation);
        void                            RenderLighting(Shader* SceneShader,Shader* EntityShader,const glm::vec3& CameraPosition);
        void                            RenderSceneLighting(Shader* SceneShader,const glm::vec3& CameraPosition);
    private:
        std::vector<LightInfomation>    WorldLightInfomation;
        glm::mat4                       LightView;
        glm::vec3                       CombinedLightValues;
};

#endif