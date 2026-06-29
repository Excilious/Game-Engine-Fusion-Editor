#ifndef __MODEL_HEADER__
#define __MODEL_HEADER__

#define MODEL_OFFSET 1

#include <map>
#include "../Debug/BoundSystem.h"
#include "../Camera.h"
#include "Mesh.h"

class Model: public Mesh
{
    public:
                                                Model(void);
        void                                    RenderModel(Shader* ShaderInstance,Camera* CameraInstance,float NearPlane,float FarPlane,float FieldOfView);
        void                                    LoadFromGLTF(Parser* EngineParser,const std::string& MeshSource,const glm::vec3& Translation,const glm::quat& Rotation,const glm::vec3& Scale);
        void                                    SetModelTexture(void);
        void                                    DestroyModel(void);

        void                                    SetModelPosition(const glm::vec3& Position);
        void                                    SetModelScale(const glm::vec3& Scale);
        void                                    SetModelRotation(const glm::quat& Rotation);

        glm::vec3                               GetModelPosition(void);
        glm::quat                               GetModelRotation(void);
        glm::vec3                               GetModelScale(void);

        BoundingBox                             EntityBounds;
        BoundSystem                             ModelBoundSystem;
        glm::mat4                               TranslationMatrix;
        glm::mat4                               RotationMatrix;
        glm::mat4                               ScaleMatrix;
        glm::vec3                               NextPosition;
    private:
        void                                    DrawModel(Shader* ShaderInstance,Camera* CameraInstance,float NearPlane,float FarPlane,float FieldOfView);
        std::vector<float>                      PositionToFloat(const std::vector<ModelVertex>& Vertices);

        std::map<unsigned int,ImageFormat>      ModelTextureMaps;
        glm::vec3                               ModelPosition;
        glm::vec3                               ModelScale;
        glm::quat                               ModelRotation;

        int                                     CounterIndex;
        unsigned int                            ModelVertexBuffer;
        unsigned int                            ModelElementBuffer;
        unsigned int                            ModelVertexArray;

        unsigned int                            ModelDepthMapFrameBuffer;
        unsigned int                            ModelDepthMap;
        unsigned int                            ShadowWidth     = 1024;
        unsigned int                            ShadowHeight    = 1024;
};     

#endif