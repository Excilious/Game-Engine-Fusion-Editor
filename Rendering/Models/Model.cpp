#include "Model.h"

Model::Model(void) {}

std::vector<float> Model::PositionToFloat(const std::vector<ModelVertex>& Vertices)
{
    std::vector<float> FloatVertices;
    for (int Index = 0; Index < Vertices.size(); Index++) 
    {
        FloatVertices.push_back(Vertices[Index].Positions.x);
        FloatVertices.push_back(Vertices[Index].Positions.y);
        FloatVertices.push_back(Vertices[Index].Positions.z);
        FloatVertices.push_back(Vertices[Index].Normals.x);
        FloatVertices.push_back(Vertices[Index].Normals.y);
        FloatVertices.push_back(Vertices[Index].Normals.z);
        FloatVertices.push_back(Vertices[Index].TextureUV.x);
        FloatVertices.push_back(Vertices[Index].TextureUV.y);
    }
    return FloatVertices;
}

glm::vec3 Model::GetModelPosition(void) {return this->ModelPosition;}
glm::quat Model::GetModelRotation(void) {return this->ModelRotation;}
glm::vec3 Model::GetModelScale(void)    {return this->ModelScale;}

void Model::SetModelPosition(const glm::vec3& NewPosition)
{
    this->TranslationMatrix     = glm::mat4(1.0f);
    this->ModelPosition         = NewPosition;
    this->TranslationMatrix     = glm::translate(this->TranslationMatrix,NewPosition);
}

void Model::SetModelScale(const glm::vec3& NewScale)
{
    this->ScaleMatrix       = glm::mat4(1.0f);
    this->ModelScale        = NewScale;
    this->ScaleMatrix       = glm::scale(this->ScaleMatrix,NewScale);
}

void Model::SetModelRotation(const glm::quat& NewRotation)
{
    this->RotationMatrix    = glm::mat4(1.0f);
    this->ModelRotation     = NewRotation;
    this->RotationMatrix    = glm::mat4_cast(NewRotation);
}

void Model::LoadFromGLTF(Parser* EngineParser,const std::string& MeshSource,const glm::vec3& Translation,const glm::quat& Rotation,const glm::vec3& Scale)
{
    if (EngineParser->GetDatapackData(MeshSource) == "") 
    {Console::SysPrint("Failed to load '"+MeshSource+"'. Model doesnt exist on file!",2); return;}

    this->LoadMeshUsingGLTF(EngineParser,MeshSource);
    if (this->GetMeshVertices().size() != 0 && this->GetMeshIndices().size() != 0)
    {
        if (this->MeshJson.find("images") != this->MeshJson.end()) 
        {
            for (int TextureIndex = 0; TextureIndex < this->MeshJson["images"].size(); TextureIndex++)
            {
                unsigned int TextureIdentifer;
                std::filesystem::path NewPath   = this->MeshJson["images"][TextureIndex]["uri"];
                ImageFormat TextureFormatImage  = EngineParser->GetImageBuffer("Textures/"+NewPath.stem().string()+".imgbuf");

                glGenTextures(1,&TextureIdentifer);
                this->ModelTextureMaps.insert({TextureIdentifer,TextureFormatImage});
            }
        }

        this->EntityBounds              = this->ModelBoundSystem.GetBoundingBoxes(this->MeshPositions, this->ModelPosition);
        this->TranslationMatrix         = glm::mat4(1.0f);
        this->RotationMatrix            = glm::mat4(1.0f);
        this->ScaleMatrix               = glm::mat4(1.0f);
        this->ModelPosition             = Translation;
        this->ModelRotation             = Rotation;
        this->ModelScale                = Scale;

        this->TranslationMatrix     = glm::translate(this->TranslationMatrix,this->ModelPosition);
        this->RotationMatrix        = glm::mat4_cast(this->ModelRotation);
        this->ScaleMatrix           = glm::scale(this->ScaleMatrix,this->ModelScale);
        this->NextPosition          = glm::vec3(0.0f,0.0f,0.0f);
        this->SetModelTexture();

        glGenVertexArrays(1,&this->ModelVertexArray);
        glBindVertexArray(this->ModelVertexArray);

        glGenBuffers(1,&this->ModelVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER,this->ModelVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER,sizeof(float)*this->PositionToFloat(this->GetMeshVertices()).size(),this->PositionToFloat(this->GetMeshVertices()).data(),GL_STATIC_DRAW);

        glGenBuffers(1,&this->ModelElementBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->ModelElementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(unsigned int)*this->GetMeshIndices().size(),this->GetMeshIndices().data(),GL_STATIC_DRAW);

        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    }
    else {Console::SysPrint("Failed to load '"+MeshSource+"' model asset.",2);}
}

void Model::SetModelTexture(void)
{
    int Counter = MODEL_OFFSET;
    for (std::map<unsigned int,ImageFormat>::iterator NewIndex = this->ModelTextureMaps.begin(); NewIndex != this->ModelTextureMaps.end(); NewIndex++)
    {
        unsigned int ChannelFormat = (NewIndex->second.NewBufferFormat.TextureChannel == 4) ? GL_RGBA :
                                     (NewIndex->second.NewBufferFormat.TextureChannel == 3) ? GL_RGB :
                                     (NewIndex->second.NewBufferFormat.TextureChannel == 1) ? GL_RED : GL_RGB; 

        glActiveTexture(GL_TEXTURE0+Counter);
        glBindTexture(GL_TEXTURE_2D,NewIndex->first);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

        glTexImage2D(
            GL_TEXTURE_2D,0,
            ChannelFormat,
            NewIndex->second.NewBufferFormat.TextureWidth,
            NewIndex->second.NewBufferFormat.TextureHeight,
            0,ChannelFormat,GL_UNSIGNED_BYTE
            ,&NewIndex->second.NewBufferFormat.TextureData[0]
        );

        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D+Counter,0);
        Counter++;
    }
}

void Model::DrawModel(Shader* ShaderInstance,Camera* CameraInstance,float NearPlane,float FarPlane,float FieldOfView)
{
    glBindVertexArray(this->ModelVertexArray);

    CounterIndex = 0;
    for (std::map<unsigned int,ImageFormat>::iterator NewIndex = this->ModelTextureMaps.begin(); NewIndex != this->ModelTextureMaps.end(); NewIndex++)
    {
        ShaderInstance->SetVector1(CounterIndex,"Texture"+CounterIndex);
        glActiveTexture(GL_TEXTURE0+CounterIndex);
        glBindTexture(GL_TEXTURE_2D,NewIndex->first);
        CounterIndex++;
    }

    ShaderInstance->ActivateShader();
    ShaderInstance->SetMatrix4(CameraInstance->ReturnProjectionMatrix(),"Projection");
    ShaderInstance->SetMatrix4(CameraInstance->ReturnViewMatrix(),"View");
    ShaderInstance->SetVector3(CameraInstance->ReturnCameraPosition(),"CameraPosition");

    ShaderInstance->SetMatrix4(this->TranslationMatrix,"Translation");
    ShaderInstance->SetMatrix4(this->RotationMatrix,"Rotation");
    ShaderInstance->SetMatrix4(this->ScaleMatrix,"Scale");

    glDrawElements(GL_TRIANGLES,this->GetMeshIndices().size(),GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
}

void Model::RenderModel(Shader* ShaderInstance,Camera* CameraInstance,float NearPlane,float FarPlane,float FieldOfView)
{
    this->DrawModel(ShaderInstance,CameraInstance,NearPlane,FarPlane,FieldOfView);
}

void Model::DestroyModel(void)
{
    glDeleteVertexArrays(1,&this->ModelVertexArray);
    glDeleteBuffers(1,&this->ModelVertexBuffer);
    glDeleteBuffers(1,&this->ModelElementBuffer);
}