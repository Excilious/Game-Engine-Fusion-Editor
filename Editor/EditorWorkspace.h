#ifndef __EDITOR_WORKSPACE_HEADER__
#define __EDITOR_WORKSPACE_HEADER__
#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <algorithm>

#include "../../Resources/glad/glad.h"
#include "../../Resources/GLFW/glfw3.h"
#include "../../Resources/glm/glm.hpp"
#include "../../Resources/glm/gtc/type_ptr.hpp"
#include "../Rendering/Debug/Debug.h"
#include "../Rendering/Debug/BoundSystem.h"

#include "EditorCompiler.h"

enum ShapeInfomation
{
	SHAPE_CUBE,
	SHAPE_SPHERE,
	SHAPE_CONE,
	SHAPE_LIGHTING,
	SHAPE_PLAYER,
	SHAPE_MESH,

	SHAPE_SKYBOX
};

struct ImageBufferToolbar
{
	unsigned int				TextureID;
	unsigned int                TextureWidth;
	unsigned int                TextureHeight;
	unsigned int                TextureChannel;
	std::vector<unsigned char>  TextureData;
};

struct ImageStructure
{
	std::string FileName;
	int ObjectIndex;
};

struct CubeInfomation
{
	std::vector<float>			Vertices;
	std::vector<unsigned int>	Indices;
	
	ShapeInfomation NewShapeInfomation;
	BoundingBox		NewBoundingBox;
	std::string		FileName = "None";
	std::string		TextureName = "None";
	std::string		SpecularName = "None";
	std::string		NormalName = "None";
	glm::vec3		PreDefinedTranslation;
	glm::quat		PreDefinedRotation;
	glm::vec3		PreDefinedScale;

    glm::mat4		TranslationMatrix;
    glm::mat4		RotationMatrix;
    glm::mat4		ScaleMatrix;

	glm::vec3		Direction = glm::vec3(0.0f,0.0f,0.0f);
	glm::vec4		ColourCube = glm::vec4(0.0f,0.0f,0.0f,1.0f);

	bool			Visible = true;
	bool			ColourUse = true;
	bool			Locked = false;

	unsigned int	MaterialIndex = 0;
	unsigned int	VertexArray;
	unsigned int	VertexBuffer;
	unsigned int	ElementBuffer;

	unsigned int	TextureID = -1;
	unsigned int	SpecularID = -1;
	unsigned int	NormalID = -1;
};

class EditorWorkspace
{
	public:
		void CreateCube(const std::vector<float>& Vertices, const std::vector<unsigned int>& Indices, glm::vec3& Translate, glm::quat& Rotate, glm::vec3& Scale, ShapeInfomation Info, unsigned int TextureID, std::string Name);
		void SetCube(CubeInfomation NewInfomation);
		void UploadDebug(Debug* DebugReference, Shader* EngineShader);
        void RenderScene(unsigned int Program, glm::mat4 Projection, glm::mat4 View, glm::vec3 Position);

		void SetColour(int Index, bool ColourVisible, glm::vec4 Colour);
		void SetVisible(int Index, bool NewVisible);
		void SetPosition(int Index, glm::vec3 Position);
		void SetRotation(int Index, glm::quat Rotation);
		void SetScale(int Index, glm::vec3 Scale);
		void SetLockedPhysics(int Index, bool Physics);
		void CompileInfomation(Parser* EngineParser, std::string NewString,glm::vec3 Position);

		void DestroyBlocks(int Index);
		void CopyMainObject(int Index);
		void PasteMainObject(int Index);
		std::vector<glm::vec3> GroupVector3(const std::vector<float>& FloatVector);
		std::vector<float> GetPositions(std::vector<float> FloatVector);

		EditorCompiler	NewFileCompiler;
		CubeInfomation	CopyToolbarMainObject;
		BoundSystem		EngineBoundSystem;
		std::vector<CubeInfomation> CubeEntityGroup;
		std::map<unsigned int, ImageBufferToolbar> TextureExtraInfo;
		glm::vec3 EntityStartPosition;

		std::vector<std::string> SkyboxValues = { "Skybox/Right.imgbuf","Skybox/Left.imgbuf","Skybox/Top.imgbuf","Skybox/Bottom.imgbuf","Skybox/Front.imgbuf","Skybox/Back.imgbuf" };
		std::vector<FusionVertex> VerticesToPosition(const std::vector<float>& Vertices, const glm::mat4& ModelMatrix, size_t Stride);

		Debug* NewDebug;
		Shader* DebugShader;
		bool Loaded = false;

};


#endif