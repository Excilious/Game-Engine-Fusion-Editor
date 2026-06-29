#ifndef __MESH_HEADER__
#define __MESH_HEADER__

#include <vector>
#include <string>

#include "../../../Resources/glm/glm.hpp"
#include "../../../Resources/jsonimport/json.h"
#include "../../Common/Parser.h"

struct ModelVertex
{
    glm::vec3 Positions;
    glm::vec3 Normals;
    glm::vec2 TextureUV;
};

class Mesh
{
    public:
                                    Mesh(void);
        void                        GenerateMesh(unsigned int ScenePositionValueIndex);
        void                        LoadMeshUsingGLTF(Parser* EngineParser,const std::string& SceneSource);
        void                        TraverseModelNodes(unsigned int NextNode,const glm::mat4& Matrix = glm::mat4(1.0f));

        std::vector<ModelVertex>    GetMeshVertices(void);
        std::vector<unsigned int>   GetMeshIndices(void);
        std::vector<unsigned char>  MeshData;
        nlohmann::json              MeshJson;

        std::vector<glm::vec3>      MeshPositions;
        std::vector<glm::vec3>      MeshNormals;
        std::vector<glm::vec2>      MeshTexture;
    private:
        std::string                 MeshContents;

        std::vector<unsigned int>   MeshIndices;
        std::vector<ModelVertex>    MeshVertices;
        glm::mat4                   MeshMatrix;

        std::vector<float>          GetFloats(const nlohmann::json& Accessor);
        std::vector<unsigned int>   GetIndices(const nlohmann::json& Accessor);
        std::vector<glm::vec2>      GroupVector2(const std::vector<float>& FloatVector);
        std::vector<glm::vec3>      GroupVector3(const std::vector<float>& FloatVector);
};


#endif