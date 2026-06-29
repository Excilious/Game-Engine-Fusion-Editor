#ifndef __DEBUG_HEADER__
#define __DEBUG_HEADER__

#include "../../../Resources/glm/glm.hpp"
#include "../../../Resources/glad/glad.h"
#include "../../Rendering/Models/Model.h"
#include "../../Common/Shader.h"

struct DebugRenderBuffer
{
    unsigned int                DebugVertexBuffer;
    unsigned int                DebugElementBuffer;
    unsigned int                DebugVertexArray;

    std::vector<unsigned int>   Indices;
    std::vector<float>          Vertices;

    glm::mat4                   Translation;
    glm::mat4                   Rotation;
    glm::mat4                   Scale;
};

class Debug
{
    public:
                                        Debug(void);
        void                            UploadOutline(Shader* DebugShader, std::vector<float> Vertices, std::vector<unsigned int> Indices, glm::vec3 Position, glm::quat Rotation, glm::vec3 Scale);
        void                            UploadVerticesBoxValues(Shader* DebugShader, glm::vec3 MinBounds, glm::vec3 MaxBounds, glm::vec3 Position, glm::quat Rotation, glm::vec3 Scale);
        void                            UploadBoundingBoxValues(Shader* DebugShader,Model* EntityModel);
        void                            RenderBoundingBoxValues(const glm::mat4& Projection,const glm::mat4& View);
        void                            RenderOneBoundingBoxValues(int Index, const glm::mat4& Projection, const glm::mat4& View, glm::mat4 Translate, glm::mat4 Rotate, glm::mat4 Scale);
        void                            DestroyDebugger(void);
        void                            RenderDebugValues(int Index, const glm::mat4& Projection, const glm::mat4& View, glm::mat4 Translate, glm::mat4 Rotate, glm::mat4 Scale);
        void                            DestroyOneBounds(int Index);

        std::vector<DebugRenderBuffer>  DebugRenderGroup;
    private:
        Shader*                         DebugGlobalShader;
        Model*                          DebugModel;
        
        glm::vec3                       Minimum;
        glm::vec3                       Maximum;

        std::vector<DebugRenderBuffer>  SceneRenderGroup;
};

#endif