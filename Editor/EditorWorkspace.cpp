
#include "EditorWorkspace.h"
#include "../Common/Console.h"

std::vector<glm::vec3> EditorWorkspace::GroupVector3(const std::vector<float>& FloatVector)
{
	std::vector<glm::vec3> Vectors;
	for (int Index = 0; Index < FloatVector.size(); Index)
	{
		Vectors.push_back(glm::vec3(FloatVector[Index++], FloatVector[Index++], FloatVector[Index++]));
	}
	return Vectors;
}

std::vector<float> EditorWorkspace::GetPositions(std::vector<float> FloatVector)
{
	std::vector<float> VerticesPosition = {};
	for (int Index = 0; Index < FloatVector.size(); Index += 8)
	{
		VerticesPosition.push_back(FloatVector[Index + 0]);
		VerticesPosition.push_back(FloatVector[Index + 1]);
		VerticesPosition.push_back(FloatVector[Index + 2]);
	}
	return VerticesPosition;
}

std::vector<FusionVertex> EditorWorkspace::VerticesToPosition(
	const std::vector<float>& Vertices,
	const glm::mat4& ModelMatrix,
	size_t Stride)
{
	std::vector<FusionVertex> PositionVertices;
	PositionVertices.reserve(Vertices.size() / Stride);

	// Precompute normal matrix
	glm::mat3 NormalMatrix = glm::transpose(glm::inverse(glm::mat3(ModelMatrix)));

	for (size_t i = 0; i < Vertices.size(); i += Stride)
	{
		FusionVertex NewVertex;

		// Transform position
		glm::vec4 WorldPos = ModelMatrix * glm::vec4(Vertices[i], Vertices[i + 1], Vertices[i + 2], 1.0f);
		NewVertex.Position = glm::vec3(WorldPos);

		// Transform normal
		NewVertex.Normal = glm::normalize(NormalMatrix * glm::vec3(Vertices[i + 3], Vertices[i + 4], Vertices[i + 5]));

		// UVs
		NewVertex.UV = glm::vec2(Vertices[i + 6], Vertices[i + 7]);

		PositionVertices.push_back(NewVertex);
	}

	return PositionVertices;
}

void EditorWorkspace::UploadDebug(Debug* DebugReference, Shader* EngineShader)
{
	this->NewDebug = DebugReference;
	this->DebugShader = EngineShader;
}

void EditorWorkspace::SetCube(CubeInfomation NewInfomation)
{
	glGenVertexArrays(1, &NewInfomation.VertexArray);
	glGenBuffers(1, &NewInfomation.VertexBuffer);
	glGenBuffers(1, &NewInfomation.ElementBuffer);

	glBindVertexArray(NewInfomation.VertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, NewInfomation.VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * NewInfomation.Vertices.size(), NewInfomation.Vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &NewInfomation.ElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewInfomation.ElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NewInfomation.Indices.size(), NewInfomation.Indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	this->NewDebug->UploadVerticesBoxValues(
		this->DebugShader,
		glm::vec3(NewInfomation.NewBoundingBox.MinBounds.x - 0.05, NewInfomation.NewBoundingBox.MinBounds.y - 0.05, NewInfomation.NewBoundingBox.MinBounds.z - 0.05),
		glm::vec3(NewInfomation.NewBoundingBox.MaxBounds.x + 0.05, NewInfomation.NewBoundingBox.MaxBounds.y + 0.05, NewInfomation.NewBoundingBox.MaxBounds.z + 0.05),
		NewInfomation.PreDefinedTranslation,
		NewInfomation.PreDefinedRotation,
		NewInfomation.PreDefinedScale
	);

	this->CubeEntityGroup.push_back(NewInfomation);
}

void EditorWorkspace::CreateCube(const std::vector<float>& Vertices, const std::vector<unsigned int>& Indices, glm::vec3& Translate, glm::quat& Rotate, glm::vec3& Scale, ShapeInfomation Info, unsigned int TextureID, std::string Name)
{
	CubeInfomation NewCubeInfomation;

	NewCubeInfomation.Vertices = Vertices;
	NewCubeInfomation.Indices = Indices;

	NewCubeInfomation.PreDefinedTranslation = Translate;
	NewCubeInfomation.PreDefinedRotation = Rotate;
	NewCubeInfomation.PreDefinedScale = Scale;
	NewCubeInfomation.NewShapeInfomation = Info;

	Translate = NewCubeInfomation.PreDefinedTranslation;
	Rotate = NewCubeInfomation.PreDefinedRotation;
	Scale = NewCubeInfomation.PreDefinedScale;

	NewCubeInfomation.TextureID = TextureID;
	NewCubeInfomation.TextureName = Name;
	NewCubeInfomation.Direction = glm::vec3(0.0f, 1.0f, 0.0f);
	NewCubeInfomation.TranslationMatrix = glm::translate(glm::mat4(1.0f), NewCubeInfomation.PreDefinedTranslation);
	NewCubeInfomation.RotationMatrix = glm::mat4_cast(glm::normalize(NewCubeInfomation.PreDefinedRotation));
	NewCubeInfomation.ScaleMatrix = glm::scale(glm::mat4(1.0f), NewCubeInfomation.PreDefinedScale);

	NewCubeInfomation.NewBoundingBox = this->EngineBoundSystem.GetBoundingBoxes(this->GroupVector3(this->GetPositions(NewCubeInfomation.Vertices)), NewCubeInfomation.PreDefinedTranslation);

	glGenVertexArrays(1, &NewCubeInfomation.VertexArray);
	glGenBuffers(1, &NewCubeInfomation.VertexBuffer);
	glGenBuffers(1, &NewCubeInfomation.ElementBuffer);

	glBindVertexArray(NewCubeInfomation.VertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, NewCubeInfomation.VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * NewCubeInfomation.Vertices.size(), NewCubeInfomation.Vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &NewCubeInfomation.ElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewCubeInfomation.ElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NewCubeInfomation.Indices.size(), NewCubeInfomation.Indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	this->NewDebug->UploadVerticesBoxValues(
		this->DebugShader,
		glm::vec3(NewCubeInfomation.NewBoundingBox.MinBounds.x - 0.05, NewCubeInfomation.NewBoundingBox.MinBounds.y - 0.05, NewCubeInfomation.NewBoundingBox.MinBounds.z - 0.05),
		glm::vec3(NewCubeInfomation.NewBoundingBox.MaxBounds.x + 0.05, NewCubeInfomation.NewBoundingBox.MaxBounds.y + 0.05, NewCubeInfomation.NewBoundingBox.MaxBounds.z + 0.05),
		NewCubeInfomation.PreDefinedTranslation,
		NewCubeInfomation.PreDefinedRotation,
		NewCubeInfomation.PreDefinedScale
	);

	this->CubeEntityGroup.push_back(NewCubeInfomation);
}

void EditorWorkspace::CopyMainObject(int Index)
{
	this->CopyToolbarMainObject = this->CubeEntityGroup[Index];
}

void EditorWorkspace::PasteMainObject(int Index)
{
	this->SetCube(this->CopyToolbarMainObject);
}

void EditorWorkspace::SetPosition(int Index, glm::vec3 Position)
{
	this->CubeEntityGroup[Index].PreDefinedTranslation = Position;
	this->CubeEntityGroup[Index].TranslationMatrix = glm::translate(glm::mat4(1.0f), Position);
}

void EditorWorkspace::SetRotation(int Index, glm::quat Rotation)
{
	this->CubeEntityGroup[Index].PreDefinedRotation = Rotation;
	this->CubeEntityGroup[Index].RotationMatrix = glm::mat4_cast(Rotation);
}

void EditorWorkspace::SetScale(int Index, glm::vec3 Scale)
{
	this->CubeEntityGroup[Index].PreDefinedScale = Scale;
	this->CubeEntityGroup[Index].ScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);
}

void EditorWorkspace::SetVisible(int Index, bool NewVisible)
{
	this->CubeEntityGroup[Index].Visible = NewVisible;
}


void EditorWorkspace::SetColour(int Index, bool ColourVisible, glm::vec4 Colour)
{
	this->CubeEntityGroup[Index].ColourUse = ColourVisible;
	this->CubeEntityGroup[Index].ColourCube = Colour;
}

void EditorWorkspace::SetLockedPhysics(int Index, bool Physics)
{
	this->CubeEntityGroup[Index].Locked = Physics;
}



void EditorWorkspace::CompileInfomation(Parser* EngineParser,std::string NewString, glm::vec3 Position)
{
	std::vector<int> AvailableTextures = {};
	DataContainer NewBlockFormat;
	std::map<unsigned int, ImageStructure> FileMap = {};
	std::vector<unsigned int> UsedID = {};

	int BlockHeader = 0x12;
	int MeshHeader = 0x12;
	int ImageHeader = 0x12;

	for (int Index = 0; Index < this->CubeEntityGroup.size(); Index++)
	{
		if (this->CubeEntityGroup[Index].NewShapeInfomation == SHAPE_LIGHTING)
		{
			FusionLightStructure NewLightStructure;

			NewLightStructure.LightPosition = this->CubeEntityGroup[Index].PreDefinedTranslation;
			NewLightStructure.LightDirection = this->CubeEntityGroup[Index].Direction;
			NewLightStructure.LightColour = this->CubeEntityGroup[Index].ColourCube;
			NewLightStructure.LightGroupSystem = LIGHT_TYPE_POINT;

			NewBlockFormat.LightGroup.push_back(NewLightStructure);
		}
		else if (this->CubeEntityGroup[Index].NewShapeInfomation == SHAPE_PLAYER)
		{
			PlayerStructure NewPlayerStructure;

			NewPlayerStructure.PlayerPosition = this->CubeEntityGroup[Index].PreDefinedTranslation;
			NewPlayerStructure.PlayerDimensions = this->CubeEntityGroup[Index].PreDefinedScale;

			NewBlockFormat.PlayerSpawns.push_back(NewPlayerStructure);
		}
		else if (this->CubeEntityGroup[Index].NewShapeInfomation == SHAPE_MESH)
		{
			ModelFile NewModelFile;
			PhysicsController NewController;
			ImageStructure NewImageStructure;

			NewController.IsPhysicsLocked = 1;

			NewModelFile.Header = MeshHeader;
			NewModelFile.BlockIndex = Index;

			NewModelFile.ModelMatrix = this->CubeEntityGroup[Index].TranslationMatrix * this->CubeEntityGroup[Index].RotationMatrix * this->CubeEntityGroup[Index].ScaleMatrix;
			NewModelFile.Position = this->CubeEntityGroup[Index].PreDefinedTranslation;
			NewModelFile.Rotation = this->CubeEntityGroup[Index].PreDefinedRotation;
			NewModelFile.Scale = this->CubeEntityGroup[Index].PreDefinedScale;

			NewImageStructure.ObjectIndex = Index;
			NewImageStructure.FileName = this->CubeEntityGroup[Index].TextureName;

			FileMap.insert({ this->CubeEntityGroup[Index].TextureID, NewImageStructure });

			NewModelFile.CurrentMaterials = (this->CubeEntityGroup[Index].MaterialIndex == 0 ? MATERIAL_NULL : MATERIAL_FLUID);
			NewModelFile.PhysController = NewController;
			NewModelFile.AnimationKeyframe = {};
			NewModelFile.Indices = this->CubeEntityGroup[Index].Indices;
			NewModelFile.Vertices = this->CubeEntityGroup[Index].Vertices;

			NewModelFile.ImageName = this->CubeEntityGroup[Index].TextureName;
			NewModelFile.FileName = this->CubeEntityGroup[Index].FileName;

			NewModelFile.AnimationLength = 0;
			NewModelFile.IndicesSize = this->CubeEntityGroup[Index].Indices.size();
			NewModelFile.VerticesSize = this->CubeEntityGroup[Index].Vertices.size();
			NewModelFile.ImageNameSize = this->CubeEntityGroup[Index].TextureName.size();
			NewModelFile.FileNameSize = this->CubeEntityGroup[Index].FileName.size();

			NewBlockFormat.EntityBlock.push_back(NewModelFile);
		}
		else if (this->CubeEntityGroup[Index].NewShapeInfomation == SHAPE_CUBE)
		{
			FusionMeshBlock NewMeshBlock;
			PhysicsController NewPhysicsController;
			ImageStructure NewImageStructure;

			NewPhysicsController.IsPhysicsLocked = 1;

			NewMeshBlock.BlockIndex = BlockHeader;
			NewMeshBlock.BlockIndex = Index;
			NewMeshBlock.BlockType = WORLD_MESH_STATIC;
			NewMeshBlock.CurrentPhysicsController = NewPhysicsController;
			NewMeshBlock.ImageFile = this->CubeEntityGroup[Index].TextureName;

			NewMeshBlock.Indices = this->CubeEntityGroup[Index].Indices;
			NewMeshBlock.Vertices = this->VerticesToPosition(
				this->CubeEntityGroup[Index].Vertices,
				this->CubeEntityGroup[Index].TranslationMatrix *
				this->CubeEntityGroup[Index].RotationMatrix *
				this->CubeEntityGroup[Index].ScaleMatrix,
				8
			);

			NewImageStructure.ObjectIndex = Index;
			NewImageStructure.FileName = this->CubeEntityGroup[Index].TextureName;

			FileMap.insert({ this->CubeEntityGroup[Index].TextureID, NewImageStructure });
			NewMeshBlock.LightmapData = {};

			NewMeshBlock.Position = this->CubeEntityGroup[Index].PreDefinedTranslation;
			NewMeshBlock.Rotation = this->CubeEntityGroup[Index].PreDefinedRotation;
			NewMeshBlock.Scale = this->CubeEntityGroup[Index].PreDefinedScale;

			NewMeshBlock.ImageFileSize = this->CubeEntityGroup[Index].TextureName.size();
			NewMeshBlock.IndicesSize = this->CubeEntityGroup[Index].Indices.size();
			NewMeshBlock.VerticesSize = NewMeshBlock.Vertices.size();
			NewMeshBlock.LightmapDataSize = 0;

			NewBlockFormat.WorldMeshBlock.push_back(NewMeshBlock);
		}
	}

	for (std::map<unsigned int, ImageStructure>::iterator NewIteratorIndex = FileMap.begin(); NewIteratorIndex != FileMap.end(); NewIteratorIndex++)
	{
		FusionGraphicalImage NewGraphicalImage;

		if (std::find(UsedID.begin(), UsedID.end(), this->CubeEntityGroup[NewIteratorIndex->second.ObjectIndex].TextureID) != UsedID.end())
		{
			continue;
		}

		NewGraphicalImage.Header = ImageHeader;
		NewGraphicalImage.ImageAmount = 1;
		NewGraphicalImage.ImageChannel = this->TextureExtraInfo.find(this->CubeEntityGroup[NewIteratorIndex->second.ObjectIndex].TextureID)->second.TextureChannel;
		NewGraphicalImage.ImageDimensions = glm::vec2(
			this->TextureExtraInfo.find(this->CubeEntityGroup[NewIteratorIndex->second.ObjectIndex].TextureID)->second.TextureWidth,
			this->TextureExtraInfo.find(this->CubeEntityGroup[NewIteratorIndex->second.ObjectIndex].TextureID)->second.TextureHeight
		);

		NewGraphicalImage.ImageContents = this->TextureExtraInfo.find(this->CubeEntityGroup[NewIteratorIndex->second.ObjectIndex].TextureID)->second.TextureData;
		NewGraphicalImage.ImageType = GRAPHICAL_IMAGE_STATIC;

		NewGraphicalImage.FileName = NewIteratorIndex->second.FileName;
		NewGraphicalImage.FileNameLength = NewIteratorIndex->second.FileName.size();

		NewGraphicalImage.ImageContentLength = NewGraphicalImage.ImageContents.size();

		NewBlockFormat.ImageGroup.push_back(NewGraphicalImage);
		UsedID.push_back(this->CubeEntityGroup[NewIteratorIndex->second.ObjectIndex].TextureID);
	}

	std::vector<ImageFormat> FormatList;
	std::vector<int> ImageIndex;
	//Right, Left, Top, Bottom, Front, Back

	for (int Index = 0; Index < this->SkyboxValues.size(); Index++)
	{
		ImageFormat NewFormat = EngineParser->GetImageBuffer(SkyboxValues[Index]);
		if (NewFormat.FormatUsed) { FormatList.push_back(NewFormat); }
		else { Console::SysPrint("Failed to get file " + SkyboxValues[Index], 2); continue; }
	}

	for (int Index = 0; Index < FormatList.size(); Index++)
	{
		FusionGraphicalImage NewImage;

		NewImage.Header = ImageHeader;
		NewImage.ImageAmount = 1;
		NewImage.ImageChannel = FormatList[Index].NewBufferFormat.TextureChannel;
		NewImage.ImageDimensions = glm::vec2(
			FormatList[Index].NewBufferFormat.TextureWidth,
			FormatList[Index].NewBufferFormat.TextureHeight
		);

		NewImage.ImageContents = FormatList[Index].ImageContents;
		NewImage.ImageType = GRAPHICAL_IMAGE_STATIC;
		NewImage.FileName = "Skybox_"+std::to_string(Index);
		NewImage.FileNameLength = NewImage.FileName.size();

		NewImage.ImageContentLength = NewImage.ImageContents.size();
	
		ImageIndex.push_back(NewBlockFormat.ImageGroup.size());
		NewBlockFormat.ImageGroup.push_back(NewImage);
	}


	//TODO: Fix this mess and add multiple skyboxes;
	FusionSkybox NewSkyboxObject;

	NewSkyboxObject.SkyboxRight = ImageIndex[0];
	NewSkyboxObject.SkyboxLeft = ImageIndex[1];
	NewSkyboxObject.SkyboxTop = ImageIndex[2];
	NewSkyboxObject.SkyboxBottom = ImageIndex[3];
	NewSkyboxObject.SkyboxFront = ImageIndex[4];
	NewSkyboxObject.SkyboxBack = ImageIndex[5];

	NewBlockFormat.SkyboxGroup.push_back(NewSkyboxObject);

	NewBlockFormat.NumberOfSkybox = NewBlockFormat.SkyboxGroup.size();
	NewBlockFormat.NumberOfPlayerSpawns = NewBlockFormat.PlayerSpawns.size();
	NewBlockFormat.NumberOfImages = NewBlockFormat.ImageGroup.size();
	NewBlockFormat.NumberOfShaders = NewBlockFormat.ShaderGroup.size();
	NewBlockFormat.NumberOfLights = NewBlockFormat.LightGroup.size();
	NewBlockFormat.NumberOfMeshBlocks = NewBlockFormat.WorldMeshBlock.size();
	NewBlockFormat.NumberOfEntityBlocks = NewBlockFormat.EntityBlock.size();

	this->NewFileCompiler.CompilePrograms(NewString, NewBlockFormat);
}

void EditorWorkspace::RenderScene(unsigned int Program, glm::mat4 Projection, glm::mat4 View, glm::vec3 Position)
{
	for (int Index = 0; Index < this->CubeEntityGroup.size(); Index++)
	{
		if (!this->CubeEntityGroup[Index].Visible) { continue; }

		this->EngineBoundSystem.UpdateBounds(this->CubeEntityGroup[Index].PreDefinedTranslation, this->CubeEntityGroup[Index].NewBoundingBox);

		glBindVertexArray(this->CubeEntityGroup[Index].VertexArray);

		if (!this->CubeEntityGroup[Index].ColourUse && this->CubeEntityGroup[Index].TextureID != -1)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->CubeEntityGroup[Index].TextureID);
			glUniform1i(glGetUniformLocation(Program, "Diffuse"), 0);
		}

		if (this->CubeEntityGroup[Index].NewShapeInfomation == SHAPE_MESH) { glFrontFace(GL_CW); }
		else { glFrontFace(GL_CCW); }

		glUseProgram(Program);
		glUniformMatrix4fv(glGetUniformLocation(Program, "Projection"), 1, GL_FALSE, glm::value_ptr(Projection));
		glUniformMatrix4fv(glGetUniformLocation(Program, "View"), 1, GL_FALSE, glm::value_ptr(View));
		glUniform3f(glGetUniformLocation(Program, "CameraPosition"), Position.x, Position.y, Position.z);

		glUniformMatrix4fv(glGetUniformLocation(Program, "Translation"), 1, GL_FALSE, glm::value_ptr(this->CubeEntityGroup[Index].TranslationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(Program, "Rotation"), 1, GL_FALSE, glm::value_ptr(this->CubeEntityGroup[Index].RotationMatrix));
		glUniformMatrix4fv(glGetUniformLocation(Program, "Scale"), 1, GL_FALSE, glm::value_ptr(this->CubeEntityGroup[Index].ScaleMatrix));

		glUniform1f(glGetUniformLocation(Program, "IsColour"), (this->CubeEntityGroup[Index].ColourUse ? 0.0f : 1.0f));
		glUniform4f(glGetUniformLocation(Program, "Colour"), this->CubeEntityGroup[Index].ColourCube.x, this->CubeEntityGroup[Index].ColourCube.y, this->CubeEntityGroup[Index].ColourCube.z, this->CubeEntityGroup[Index].ColourCube.w);

		glDrawElements(GL_TRIANGLES, this->CubeEntityGroup[Index].Indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}

void EditorWorkspace::DestroyBlocks(int Index)
{
	if (this->CubeEntityGroup.size() < Index) { return; }


	glDeleteVertexArrays(1, &this->CubeEntityGroup[Index].VertexArray);
	glDeleteBuffers(1, &this->CubeEntityGroup[Index].VertexBuffer);
	glDeleteBuffers(1, &this->CubeEntityGroup[Index].ElementBuffer);

	this->NewDebug->DestroyOneBounds(Index);
	this->CubeEntityGroup.erase(this->CubeEntityGroup.begin() + Index);
}
