#include "Mesh.h"

Mesh::Mesh(void) {}

std::vector<ModelVertex> Mesh::GetMeshVertices(void)    {return this->MeshVertices;}
std::vector<unsigned int> Mesh::GetMeshIndices(void)    {return this->MeshIndices;}

std::vector<glm::vec3> Mesh::GroupVector3(const std::vector<float>& FloatVector)
{
    std::vector<glm::vec3> Vectors;
    for (int Index = 0; Index < FloatVector.size(); Index) {Vectors.push_back(glm::vec3(FloatVector[Index++],FloatVector[Index++],FloatVector[Index++]));}
    return Vectors;
}

std::vector<glm::vec2> Mesh::GroupVector2(const std::vector<float>& FloatVector)
{
    std::vector<glm::vec2> Vectors;
    for (int Index = 0; Index < FloatVector.size(); Index) {Vectors.push_back(glm::vec2(FloatVector[Index++],FloatVector[Index++]));}
    return Vectors;
}

std::vector<unsigned int> Mesh::GetIndices(const nlohmann::json& Accessor)
{
    std::vector<GLuint> Indices;
    unsigned int BufferViewIndices      = Accessor.value("bufferView",0);
    unsigned int Count                  = Accessor["count"];
    unsigned int AccessorByteOffset     = Accessor.value("byteOffset",0);
    unsigned int ComponentType          = Accessor["componentType"];

    nlohmann::json BufferView           = this->MeshJson["bufferViews"][BufferViewIndices];
    unsigned int ByteOffset             = BufferView["byteOffset"];
    unsigned int StartOfData            = ByteOffset+AccessorByteOffset;

    if (ComponentType == 5125)
    {
        for (unsigned int Index = StartOfData; Index < ByteOffset+AccessorByteOffset+Count*4; Index)
        {
            unsigned char Bytes[] = {this->MeshData[Index++],this->MeshData[Index++],this->MeshData[Index++],this->MeshData[Index++]};
            unsigned int Value;
            std::memcpy(&Value,Bytes,sizeof(unsigned int));
            Indices.push_back((GLuint)Value);
        }
    }
    else if (ComponentType == 5123)
    {
        for (unsigned int Index = StartOfData; Index < ByteOffset+AccessorByteOffset+Count*2; Index)
        {
            unsigned char Bytes[] = {this->MeshData[Index++],this->MeshData[Index++]};
            unsigned short Value;
            std::memcpy(&Value,Bytes,sizeof(unsigned short));
            Indices.push_back((GLuint)Value);
        }
    }
    else if (ComponentType == 5122)
    {
        for (unsigned int Index = StartOfData; Index < ByteOffset+AccessorByteOffset+Count*2; Index)
        {
            unsigned char Bytes[] = {this->MeshData[Index++],this->MeshData[Index++]};
            short Value;
            std::memcpy(&Value,Bytes,sizeof(short));
            Indices.push_back((GLuint)Value);
        }
    }
    else
    {Console::SysPrint("Failed to find ComponentType",2);}
    return Indices;
}

std::vector<float> Mesh::GetFloats(const nlohmann::json& Accessor)
{
    unsigned int BufferViewIndex        = Accessor.value("bufferView",1);
    unsigned int Count                  = Accessor["count"];
    unsigned int AccessorByteOffset     = Accessor.value("byteOffset",0);

    std::vector<float> FloatVector;
    std::string Type                    = Accessor["type"];
    nlohmann::json BufferView           = this->MeshJson["bufferViews"][BufferViewIndex];
    unsigned int ByteOffset             = BufferView["byteOffset"];
    unsigned int NumberPerVertices;

    if (Type == "SCALAR")       {NumberPerVertices=1;}
    else if (Type == "VEC2")    {NumberPerVertices=2;}
    else if (Type == "VEC3")    {NumberPerVertices=3;}
    else if (Type == "VEC4")    {NumberPerVertices=4;}
    else {Console::SysPrint("Vector doesn't exist for model.",2);}

    unsigned int StartOfData    = ByteOffset + AccessorByteOffset;
    unsigned int LengthOfData   = Count * 4 * NumberPerVertices;

    for (unsigned int Index = StartOfData; Index < StartOfData+LengthOfData; Index)
    {
        float Value;
        unsigned char Bytes[] = {this->MeshData[Index++],this->MeshData[Index++],this->MeshData[Index++],this->MeshData[Index++]};
        std::memcpy(&Value,Bytes,sizeof(float)); 
        FloatVector.push_back(Value);
    }
    return FloatVector;
}

void Mesh::TraverseModelNodes(unsigned int NextNode,const glm::mat4& Matrix)
{
    nlohmann::json NewNode = this->MeshJson["nodes"][NextNode];
    if (NewNode.find("mesh") != NewNode.end())      {this->GenerateMesh(NewNode["mesh"]);}
    if (NewNode.find("children") != NewNode.end())  {for (unsigned int Index = 0; Index < NewNode["children"].size(); Index++) {this->TraverseModelNodes(NewNode["children"][Index],glm::mat4(1.0f));}}
}

void Mesh::LoadMeshUsingGLTF(Parser* EngineParser,const std::string& SceneSource)
{
    this->MeshContents  = EngineParser->GetDatapackData(SceneSource);
    this->MeshJson      = nlohmann::json::parse(this->MeshContents);
    this->MeshData      = EngineParser->GetURIData(this->MeshContents);

    this->TraverseModelNodes(0);
}

void Mesh::GenerateMesh(unsigned int MeshPositionValueIndex)
{
    unsigned int MeshPositionAccessorIndex = this->MeshJson["meshes"][MeshPositionValueIndex]["primitives"][0]["attributes"]["POSITION"];
    unsigned int MeshNormalAccessorIndex   = this->MeshJson["meshes"][MeshPositionValueIndex]["primitives"][0]["attributes"]["NORMAL"];
    unsigned int MeshTextureAccessorIndex  = this->MeshJson["meshes"][MeshPositionValueIndex]["primitives"][0]["attributes"]["TEXCOORD_0"];
    unsigned int MeshIndicesAccessorIndex  = this->MeshJson["meshes"][MeshPositionValueIndex]["primitives"][0]["indices"];

    std::vector<float> PositionVector       = this->GetFloats(this->MeshJson["accessors"][MeshPositionAccessorIndex]);
    std::vector<float> NormalVector         = this->GetFloats(this->MeshJson["accessors"][MeshNormalAccessorIndex]);
    std::vector<float> TextureVector        = this->GetFloats(this->MeshJson["accessors"][MeshTextureAccessorIndex]);

    this->MeshPositions                    = this->GroupVector3(PositionVector);
    this->MeshNormals                      = this->GroupVector3(NormalVector);
    this->MeshTexture                      = this->GroupVector2(TextureVector);
    this->MeshIndices                      = this->GetIndices(this->MeshJson["accessors"][MeshIndicesAccessorIndex]);

    for (int Index = 0; Index < MeshPositions.size(); Index++)
    {
        this->MeshVertices.push_back(ModelVertex{ MeshPositions[Index],MeshNormals[Index],MeshTexture[Index]});
    }
}

