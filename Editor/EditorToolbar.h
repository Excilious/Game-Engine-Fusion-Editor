#ifndef __EDITOR_TOOLBAR_HEADER__
#define __EDITOR_TOOLBAR_HEADER__

#define NOMINMAX

#define SET_AS_ALPHA 

#include <regex>
#include <map>
#include <iostream>
#include <windows.h>
#include <shobjidl.h> 
#include <algorithm>

#include "EditorModelViewport.h"
#include "EditorWorkspace.h"
#include "../Common/Console.h"
#include "../Common/Parser.h"
#include "../Rendering/Debug/BoundSystem.h"
#include "../../Resources/zip/miniz.h"
#include "../../Resources/jsonimport/json.h"

#include "../../Resources/imgui/imgui.h"
#include "../../Resources/imgui/imgui_impl_opengl3.h"
#include "../../Resources/imgui/imgui_impl_glfw.h"
#include "../../Resources/imgui/imgui_stdlib.h"
#include "../../Resources/glad/glad.h"
#include "../../Resources/glm/glm.hpp"
#include "../../Resources/GLFW/glfw3.h"

enum DataType
{
    DATATYPE_NULL = 0,
    DATATYPE_ENVIROMENT_NOINTERACT,
    DATATYPE_BLOCK,
    DATATYPE_LIGHT,
    DATATYPE_PLAYER,
    DATATYPE_SKY,
    DATATYPE_SUN,
    DATATYPE_CAMERA
};

enum StatusMode
{
    STATUS_DATA_NULL = 0,
    STATUS_LOAD_DATAPACK,
    STATUS_LOAD_WORLD,
    STATUS_LOAD_IMAGE,
    STATUS_SAVE_WORLD
};

enum Material
{
    WATER = 0,
    METAL
};

struct ModelGroup
{
    int Index;
    bool Selected = false;
    bool Visible = true;
    bool UseColour = true;
    bool PhysicsLocked = true;

    std::string LabelName;
    std::string LabelPrefix;
    DataType NewDataType;
    DataType ExtraDataType;

    glm::vec3 Translate;
    glm::quat Rotate;
    glm::vec3 Scale;
    
    glm::vec4 Colour = glm::vec4(1.0f,0.0f,0.0f,1.0f);
};

enum TooltipValues
{
    VALUE_POINTER,
    VALUE_TRANSLATE,
    VALUE_ROTATE,
    VALUE_SCALE
};

class EditorToolbar
{
    public:
                EditorToolbar(void);
        void    RenderToolbar(GLFWwindow* EngineWindow,Parser* EngineParser, Camera* EngineCamera, glm::mat4 Projection, glm::mat4 View, glm::vec3 Position, unsigned int Program);
        void    RenderMainToolbar(Parser* EngineParser, Camera* EngineCamera);
        void    RenderToolbarUI(Parser* EngineParser, Camera* EngineCamera);
        void    RenderTextureViewer(Camera* EngineCamera, Parser* EngineParser);
        void    RenderWorldView(void);
        void    PromptFileExplorerView(const char* Filter, const char* FileName,StatusMode ResourceDataMode);
        void    PrintAllFiles(const std::string& File);
        void    LoadFontImages(void);
        void    RenderMeshProperties(void);
        void    RenderImageCompiler(void);
        unsigned int LoadTextureFromMemory(ImageBufferToolbar ImageFormatOut);
        void    LoadFromBuffer(const char* Buffer, size_t BufferSize, ImageBufferToolbar* ImageFormatOut);
        void    LoadNewWorld(std::string FileName);
        void    BuildObjects(int Index, DataContainer NewFormat, DataType NewDataType);

        void DrawDropdownMenu(std::vector<ModelGroup> Vector);
        void SpawnObject(ShapeInfomation ShapeInfo, std::string Name, std::string Prefix, DataType Object, glm::vec3 Position, glm::quat Rotation, glm::vec3 Scale, std::vector<float> Vertices, std::vector<unsigned int> Indices);
        void RenderSkyboxProperties(void);

        void RenderSunProperties(void);
        void RenderSettingsMenu(void);
        void CopyObject(int Index);
        void PasteObject(int Index);
        void DeleteUIBlocks(int Index);
        void RenderGeometryProperties(void);
        void RenderCameraProperties(void);
        void CreateAddMenu(void);
        void EditTextureObject(int ActionObject, glm::vec2 TextureData);
        std::vector<float> PositionToFloat(const std::vector<ModelVertex>& Vertices);

        std::string WideToString(const std::wstring& String);
        std::string ShowSaveFileDialog();
        std::string LastKnownName;

        ImageBufferFormat ReadFiles(std::string File);
        ModelGroup CurrentCopyToolbar;
        EditorWorkspace NewWorkspace;
        EditorModelViewport NewModelViewport;
        TooltipValues NewValues;
        int CurrentSelectedIndex = -1;
        int CurrentModelIndex = 0;
        int CurrentSkybox;

        bool ShowCameraProperties;
        bool SystemLoaded;
        bool Saved;

        bool SunPropertiesActive;
        bool SettingsMenuOpen;
        bool IsModelMenuOpen;
        bool GridEnable;
        bool ChangeSkyboxValue;
        int SkyboxIndexChange; 

        std::vector<std::string> Dimensions = { "Right","Left","Top","Bottom","Front","Back" };
        std::vector<ModelGroup> NewModelGroup;
        std::vector<ModelGroup> EnviromentViewGroup;
        void SetupImages();
    private:
        bool LoadImageBufferFromFile(const std::string& FilePath, ImageBufferToolbar* OutImage);
        unsigned int TranslateImageID;
        unsigned int RotateImageID;
        unsigned int ScaleImageID;

        unsigned int LogoImageID;
        unsigned int ThumbnailImageID;
        unsigned int MissingImageID;

        int SelectedItems = 0;
        int SelectedMaterial = 0;
        bool IsTextureMenuOpen;
        bool IsImageCompilerOpen;
        bool IsAddMenuOpen = false;
        std::vector<std::string> GroupItems = {"Cube","Sphere","Cone"};
        std::vector<Mesh> MeshGroup;
        std::string SearchSystem;

        mz_zip_archive NewDataparser;
        std::map<std::string, unsigned int> TextureMapIndex;
        std::map<std::string, unsigned int> SkyboxMapIndex;
        std::map<std::string, unsigned int> ModelIndex;
        std::string FileSaveName;
        std::vector<glm::vec2> TextureOffset;
};

#endif