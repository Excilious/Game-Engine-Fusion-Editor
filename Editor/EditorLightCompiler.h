#ifndef __LIGHT_COMPILER__
#define __LIGHT_COMPILER__

#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <string>
#include "../../Resources/glm/glm.hpp"

#include "../../Resources/glm/gtc/type_ptr.hpp"
#include "../../Resources/glm/gtx/matrix_decompose.hpp"


enum LightType
{
	TYPE_POINT,
	TYPE_SURFACE
};

enum FusionShaderType
{
	SHADER_TYPE_NULL = 0,
	SHADER_TYPE_VERTEX,
	SHADER_TYPE_FRAGMENT
};

enum FusionGraphicalImageType
{
	GRAPHICAL_IMAGE_NULL = 0,
	GRAPHICAL_IMAGE_STATIC,
	GRAPHICAL_IMAGE_ANIMATED
};

enum FusionWorldType
{
	WORLD_MESH_NULL = 0,
	WORLD_MESH_STATIC,
	WORLD_MESH_DYNAMIC
};

enum FusionLightType
{
	LIGHT_TYPE_NULL = 0,
	LIGHT_TYPE_POINT,
	LIGHT_TYPE_SURFACE
};

enum FusionMaterials
{
	MATERIAL_NULL = 0,
	MATERIAL_FLUID,
};

struct RayLine
{
	glm::vec3 origin;
	glm::vec3 dir;
};

struct ObjectBounds
{
	glm::vec3 BoundCenter;
	glm::vec3 MinimumBounds;
	glm::vec3 MaximumBounds;
};

struct BVHNodeSerialized
{
	ObjectBounds Bounds;

	int LeftIndex;
	int RightIndex;

	int ObjectStart;
	int ObjectCount;
};

struct BlockStructureBVH
{
	ObjectBounds						LocalBounds;
	int									BlockIndex;
};


struct BVHCollisionNode
{
	std::vector<BlockStructureBVH*>		ClusterBounds;
	ObjectBounds						BoundingBox;

	BVHCollisionNode* LeftPointer = nullptr;
	BVHCollisionNode* RightPointer = nullptr;

	int StartIndex = 0;
	int CountIndex = 0;
};

struct BlockData
{
	std::vector<BVHNodeSerialized>	NewSterilised = {};
	std::vector<int>				IndexArray = {};
};

struct FusionShaderFile
{
	int		Header;
	int		ShaderNameL;
	int		ShaderContentsL;
	FusionShaderType CurrentType;
	std::string ShaderName;
	std::string ShaderContents;
};

struct FusionGraphicalImage
{
	int Header;
	int ImageAmount;
	int ImageChannel;
	int	FileNameLength;
	int ImageContentLength;
	glm::vec2 ImageDimensions;
	FusionGraphicalImageType ImageType;
	std::string FileName;
	std::vector<unsigned char> ImageContents;
};

struct PhysicsController
{
	int IsPhysicsLocked;
};

struct FusionVertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 UV;
};

struct FusionLightStructure
{
	FusionLightType LightGroupSystem;
	glm::vec3 LightPosition;
	glm::vec3 LightColour;
	glm::vec3 LightDirection;
};

struct PlayerStructure
{
	glm::vec3 PlayerPosition;
	glm::vec3 PlayerDimensions;
};

struct BlockStructureFormat
{
	std::vector<float>			Vertices;
	std::vector<unsigned int>	Indices;

	glm::vec4					BlockColour;
	int							UseBlockColour;
	int							PhysicsLocked;
	int							BlockIndex;

	unsigned int				TextureIndex;
	unsigned int				MaterialIndex;
	ObjectBounds				LocalBounds;

	glm::vec3					Position;
	glm::quat					Rotation;
	glm::vec3					Scale;

	glm::mat4					TranslationMatrix;
	glm::mat4					RotationMatrix;
	glm::mat4					ScaleMatrix;
};

struct ModelFile
{
	int Header;
	int BlockIndex;
	int AnimationLength;
	int IndicesSize;
	int VerticesSize;
	int ImageNameSize;
	int FileNameSize;
	glm::mat4 ModelMatrix;
	glm::vec3 Position;
	glm::quat Rotation;
	glm::vec3 Scale;
	FusionMaterials CurrentMaterials;
	PhysicsController PhysController;

	std::vector<int> AnimationKeyframe;
	std::vector<unsigned int> Indices;
	std::vector<float> Vertices;
	std::string			ImageName;
	std::string			FileName;
};

struct FusionSkybox
{
	int SkyboxLeft;
	int SkyboxRight;
	int SkyboxTop;
	int SkyboxBottom;
	int SkyboxFront;
	int SkyboxBack;
};

struct FusionMeshBlock
{
	int Header;
	int BlockIndex;
	int ImageFileSize;
	int IndicesSize;
	int VerticesSize;
	int LightmapDataSize;
	FusionWorldType BlockType;
	PhysicsController CurrentPhysicsController;
	FusionMaterials             MaterialIndex;

	std::string                 ImageFile;
	std::vector<unsigned int>   Indices;
	std::vector<FusionVertex>   Vertices;
	std::vector<unsigned char>  LightmapData;

	glm::vec3					Position;
	glm::quat					Rotation;
	glm::vec3					Scale;
};

struct DataContainer
{
	size_t                            NumberOfTreeNodes;
	size_t								NumberOfTreeIndex;
	size_t                            NumberOfPlayerSpawns;
	size_t                            NumberOfImages;
	size_t                            NumberOfShaders;
	size_t							  NumberOfSkybox; //This can be changed for each scene.
	size_t                            NumberOfLights;
	size_t                            NumberOfMeshBlocks;
	size_t                            NumberOfEntityBlocks;

	BlockData							CollisionBlockData;
	std::vector<PlayerStructure> PlayerSpawns;
	std::vector<FusionGraphicalImage> ImageGroup;
	std::vector<FusionShaderFile> ShaderGroup;
	std::vector<FusionSkybox> SkyboxGroup;
	std::vector<FusionLightStructure> LightGroup;
	std::vector<FusionMeshBlock> WorldMeshBlock;
	std::vector<ModelFile> EntityBlock;
};

struct LightBlock
{
	glm::vec3 Position;
	glm::vec3 Colour;
};

class FusionLightCompiler
{
	public:
		FusionLightCompiler(void);
		void CompileToImage(std::vector<LightBlock> PositionToLight);
		void SubmitLightSurfaces(const std::vector<FusionMeshBlock>& Blocks,const std::vector<FusionLightStructure>& lights, const BVHCollisionNode* bvh);
		bool IntersectBVH(const BVHCollisionNode* node, const RayLine& ray, float maxDist);
		bool RayAABB(const RayLine& ray, const glm::vec3& minB, const glm::vec3& maxB);
		bool InShadow(glm::vec3 start, glm::vec3 lightPos, const BVHCollisionNode* bvh);
		glm::vec3 ComputeLighting(glm::vec3 pos, glm::vec3 normal, const std::vector<FusionLightStructure>& lights, const BVHCollisionNode* bvh);

};

#endif