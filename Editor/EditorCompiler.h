#ifndef __EDITOR_COMPILER_HEADER__
#define __EDITOR_COMPILER_HEADER__

#define GLM_ENABLE_EXPERIMENTAL

#include <fstream>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <string>

#include "../../Resources/glm/glm.hpp"
#include "../../Resources/glm/gtc/type_ptr.hpp"
#include "../../Resources/glm/gtx/matrix_decompose.hpp"

#include "EditorLightCompiler.h"

#define BOUNDS_EPSILON	0.25f
#define FILE_VERSION 0x151

class EditorCompiler
{
public:
	EditorCompiler(void);
	std::vector<glm::vec3>	VerticesToPosition(const std::vector<float>& Vertices, const glm::mat4& ModelMatrix, size_t Stride);
	BlockData				CompileIntoBVHTree(DataContainer NewFormat);
	void					CompilePrograms(const std::string& FileName, DataContainer& NewFormat);
	int						SteriliseTree(BVHCollisionNode* Node, std::vector<BVHNodeSerialized>& ReturnNodes, std::vector<int>& ObjectIndices);

	ObjectBounds			GenerateNewBounds(const std::vector<FusionVertex>& PositionVertices, const glm::vec3& Position);
	DataContainer			ReadFile(const char* File);
	BVHCollisionNode* CreateCollisionNode(std::vector<BlockStructureFormat*>& Blocks, int Start, int End, int Depth = 0);
	ObjectBounds			BuildUnion(ObjectBounds BoundsA, ObjectBounds BoundsB);
private:
	std::vector<BVHCollisionNode> Nodes;
	FusionLightCompiler LightCompiler;
	BVHCollisionNode* RootNode = nullptr;
};

#endif