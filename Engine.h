#ifndef __ENGINE_HEADER__
#define __ENGINE_HEADER__

#include "Session/SessionManager.h"
#include "Editor/EditorModelViewport.h"
#include "Rendering/Debug/Debug.h"

#include "../Resources/glm/gtx/matrix_decompose.hpp"
#include "../Resources/imgui/ImGuizmo.h"

struct PointEditor
{
    glm::vec3 Position;
    glm::vec3 Normal;
};

struct Vertex {
    glm::vec3 Position;
};

struct MeshBlock
{
    std::vector<PointEditor> Vertices;
    std::vector<unsigned int> Indices;
};

struct Ray
{
    glm::vec3 Origin;
    glm::vec3 Direction;
};

class Engine
{
    public:
                                    Engine(void);
        void                        StartEngine(void);
        void                        ShowEditGizmo(int ViewportWidth, int ViewportHeight);
        void                        RenderEngine(void);
        void                        CloseEngine(void);
        void                        SpecialKeyFunction(void);

        glm::vec3                   GetRayFromMouse(const glm::mat4& Projection, const glm::mat4& View, double MouseX, double MouseY, int ScreenWidth, int ScreenHeight);
        bool                        IntersectRayAABB(const glm::vec3& RayOrigin, const glm::vec3& RayDirection, const glm::vec3& BoxMin, const glm::vec3& BoxMax, float& T);
        bool                        IntersectRayTriangle(const glm::vec3& Origin, const glm::vec3& Direction, const glm::vec3& TriangleV0, const glm::vec3& TriangleV1, const glm::vec3& TriangleV2, float& Triangle);
    private:
        double                      LastNormalTrigger;
        bool                        SceneActive;
        bool                        SceneLoaded;
        int                         SelectedVertex;
        int                         LastVertex;
            

        int                         PickTriangleFromMesh(const std::vector<float>& Vertices, const std::vector<unsigned int>& Indices, glm::vec3 RauOrigin, glm::vec3 RayDirection);
        Ray                         GetRayandDirectionFromMouse(const glm::mat4& Projection, const glm::mat4& View, double MouseX, double MouseY, int ScreenWidth, int ScreenHeight);
        std::vector<PointEditor>    ConvertToPosition(const std::vector<float> Vertices);
        std::vector<float>          ConvertToVertex(const std::vector<PointEditor> Vertices, std::vector<float> OldVertices);
        std::vector<int>            GetTriangleFaces(std::vector<float> Vertices, std::vector<unsigned int> Indices, int TriangleIndex);
        glm::vec3                   GetTriangleNormal(glm::vec3 Vertex0, glm::vec3 Vertex1, glm::vec3 Vertex2);
        glm::vec3                   GetVertex(std::vector<PointEditor> Vertices, int Index);

        std::vector<MeshBlock>      MeshGroup;
        Debug                       NewDebug;
        SessionManager              NewSessionManager;
		EditorModelViewport         NewEditorViewport;

        std::vector<PointEditor>    EditablePoints;
        bool                        PointsInitialized = false;

        glm::vec3                   ObjectScale;
        glm::vec3                   ObjectTranslation;
        glm::quat                   ObjectRotation;
        glm::vec3                   Skew;
        glm::vec4                   Perspective;
};

#endif