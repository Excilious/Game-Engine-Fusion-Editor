#define STB_IMAGE_IMPLEMENTATION

#include "Skybox.h"
#include "../../Resources/stb/stb_image.h"

Skybox::Skybox(void) {}

const std::vector<float> SkyboxVertices = {
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f
};

const std::vector<unsigned int> SkyboxIndices = {
	1, 2, 6 ,6, 5, 1 ,0, 4, 7,
	7, 3, 0 ,4, 5, 6 ,6, 7, 4,
	0, 3, 2 ,2, 1, 0 ,0, 1, 5,
	5, 4, 0 ,3, 7, 6 ,6, 2, 3
};

void Skybox::SpawnSkybox(int WindowWidth,int WindowHeight,float SkyNear,float SkyFar,Parser* EngineParser,float FieldOfView)
{
    this->Width         = WindowWidth;
    this->Height        = WindowHeight;
    this->Near          = SkyNear;
    this->Far           = SkyFar;
    this->FieldOfView   = FieldOfView;

    glGenVertexArrays(1,&this->SkyboxVertexArrayObject);
    glBindVertexArray(this->SkyboxVertexArrayObject);

    glGenBuffers(1,&this->SkyboxVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER,this->SkyboxVertexBufferObject);    
    glBufferData(GL_ARRAY_BUFFER,SkyboxVertices.size()*sizeof(float),SkyboxVertices.data(),GL_STATIC_DRAW);
    
    glGenBuffers(1,&this->SkyboxVertexElementObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->SkyboxVertexElementObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,SkyboxIndices.size()*sizeof(unsigned int),SkyboxIndices.data(),GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    glGenTextures(1,&this->CubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP,this->CubemapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    
    this->GenerateSkyboxBuffers(EngineParser,{"Skybox/Right.imgbuf","Skybox/Left.imgbuf","Skybox/Top.imgbuf","Skybox/Bottom.imgbuf","Skybox/Front.imgbuf","Skybox/Back.imgbuf"});
    for (unsigned int Index = 0; Index < this->ImageBuffers.size(); Index++)
    {
        if (this->ImageBuffers[Index].FormatUsed)
        {
            unsigned int ChannelFormat = (this->ImageBuffers[Index].NewBufferFormat.TextureChannel == 4) ? GL_RGBA :
                                         (this->ImageBuffers[Index].NewBufferFormat.TextureChannel == 3) ? GL_RGB :
                                         (this->ImageBuffers[Index].NewBufferFormat.TextureChannel == 1) ? GL_RED : GL_RGB;  
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X+Index,
                0,ChannelFormat,
                this->ImageBuffers[Index].NewBufferFormat.TextureWidth,
                this->ImageBuffers[Index].NewBufferFormat.TextureHeight,
                0,ChannelFormat,GL_UNSIGNED_BYTE,
                &this->ImageBuffers[Index].ImageContents[0]
            );
        }
    }
}

void Skybox::UpdateSkyboxBuffers(Parser* EngineParser, std::vector<std::string> SkyboxShapes)
{
    this->ImageBuffers = {};
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->CubemapTexture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    this->GenerateSkyboxBuffers(EngineParser,SkyboxShapes);
    for (unsigned int Index = 0; Index < this->ImageBuffers.size(); Index++)
    {
        if (this->ImageBuffers[Index].FormatUsed)
        {
            unsigned int ChannelFormat = (this->ImageBuffers[Index].NewBufferFormat.TextureChannel == 4) ? GL_RGBA :
                (this->ImageBuffers[Index].NewBufferFormat.TextureChannel == 3) ? GL_RGB :
                (this->ImageBuffers[Index].NewBufferFormat.TextureChannel == 1) ? GL_RED : GL_RGB;
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + Index,
                0, ChannelFormat,
                this->ImageBuffers[Index].NewBufferFormat.TextureWidth,
                this->ImageBuffers[Index].NewBufferFormat.TextureHeight,
                0, ChannelFormat, GL_UNSIGNED_BYTE,
                &this->ImageBuffers[Index].ImageContents[0]
            );
        }
    }
}

void Skybox::GenerateSkyboxBuffers(Parser* EngineParser,std::vector<std::string> SkyboxShapes)
{
    for (int Index = 0; Index < SkyboxShapes.size(); Index++)
    {
        ImageFormat NewFormat = EngineParser->GetImageBuffer(SkyboxShapes[Index]);
        if (NewFormat.FormatUsed)   {this->ImageBuffers.push_back(NewFormat);}
        else                        {Console::SysPrint("Failed to get file "+SkyboxShapes[Index],2); continue;}
    }
}

void Skybox::RenderSkybox(Camera* EngineCamera,Shader* SkyboxShader,float FieldOfView)
{
    glDepthFunc(GL_LEQUAL);

    glActiveTexture(GL_TEXTURE0);
    SkyboxShader->ActivateShader();
    SkyboxShader->SetMatrix4(glm::mat4(glm::mat3(glm::lookAt(EngineCamera->ReturnCameraPosition(),EngineCamera->ReturnCameraPosition()+EngineCamera->Orientation,EngineCamera->UpPosition))),"View");
    SkyboxShader->SetMatrix4(glm::perspective(glm::radians(FieldOfView),(float)EngineCamera->Width/EngineCamera->Height,(float)EngineCamera->Near,(float)EngineCamera->Far),"Projection");
    SkyboxShader->SetVector1(0.0f,"skybox");

    glBindVertexArray(this->SkyboxVertexArrayObject);
    glBindTexture(GL_TEXTURE_CUBE_MAP,this->CubemapTexture);
    glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void Skybox::DestroySkybox(void)
{
    glDeleteBuffers(1,&this->SkyboxVertexBufferObject);
    glDeleteBuffers(1,&this->SkyboxVertexElementObject);
    glDeleteVertexArrays(1,&this->SkyboxVertexArrayObject);
}