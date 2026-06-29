#include "Lighting.h"

Lighting::Lighting(void) 
{
    this->WorldLightInfomation  = {};
    this->LightView             = glm::mat4(1.0f);
    this->CombinedLightValues   = glm::vec3(0.0f);
};

void Lighting::UploadLightingToScene(LightInfomation CurrentLightInfomation)
{
    this->CombinedLightValues = glm::vec3(0.0f,0.0f,0.0f);
    this->WorldLightInfomation.push_back(CurrentLightInfomation);
}

void Lighting::RenderLighting(Shader* SceneShader,Shader* EntityShader,const glm::vec3& CameraPosition)
{
    this->RenderSceneLighting(SceneShader,CameraPosition);
    this->RenderSceneLighting(EntityShader,CameraPosition);
}

void Lighting::RenderSceneLighting(Shader* SceneShader,const glm::vec3& CameraPosition)
{
    SceneShader->ActivateShader();
    SceneShader->SetVector3(CameraPosition,"v_CameraPosition");
    SceneShader->SetVector1(this->WorldLightInfomation.size(),"LightAmount");

    for (int Index = 0; Index < this->WorldLightInfomation.size(); Index++)
    {
        SceneShader->SetVector3(this->WorldLightInfomation[Index].LightPosition,"NewLight["+std::to_string(Index)+"].Position");
        SceneShader->SetVector3(this->WorldLightInfomation[Index].LightColour,"NewLight["+std::to_string(Index)+"].LightColour");
        SceneShader->SetVector3(
            this->WorldLightInfomation[Index].LightModel->GetModelRotation()*this->WorldLightInfomation[Index].LightDirection,
            "NewLight["+std::to_string(Index)+"].Direction"
        );

        SceneShader->SetVector1(this->WorldLightInfomation[Index].LightAmbience,"NewLight["+std::to_string(Index)+"].Ambience");
        SceneShader->SetVector1(this->WorldLightInfomation[Index].LightSpecular,"NewLight["+std::to_string(Index)+"].Specular");
        SceneShader->SetVector1(this->WorldLightInfomation[Index].LightSpecularStrength,"NewLight["+std::to_string(Index)+"].SpecularStrength");
        SceneShader->SetVector1(this->WorldLightInfomation[Index].LightBrightnes,"NewLight["+std::to_string(Index)+"].Brightness");

        SceneShader->SetVector1(this->WorldLightInfomation[Index].LightRadius,"NewLight["+std::to_string(Index)+"].Radius");
        SceneShader->SetVector1(this->WorldLightInfomation[Index].LightLinear,"NewLight["+std::to_string(Index)+"].Linear");
        SceneShader->SetVector1(this->WorldLightInfomation[Index].LightQuadratic,"NewLight["+std::to_string(Index)+"].Quadratic");
        SceneShader->SetVector1((float)this->WorldLightInfomation[Index].LightCutoff,"NewLight["+std::to_string(Index)+"].Cutoff");
        SceneShader->SetVector1((float)this->WorldLightInfomation[Index].LightOuterCutoff,"NewLight["+std::to_string(Index)+"].OuterCutoff");
    }
}