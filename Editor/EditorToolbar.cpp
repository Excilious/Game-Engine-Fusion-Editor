#define MINIZ_IMPLEMENTATION
#define MINIZ_HEADER_FILE_ONLY

#include "../../Resources/IconsFontAwesome5.h"
#include "../../Resources/stb/stb_image.h"

#include "EditorToolbar.h"
#include "../Common/Console.h"

#include "Templates/SHAPE_CubeVertices.h"
#include "Templates/SHAPE_CubeIndices.h"
#include "Templates/SHAPE_ConeVertices.h"
#include "Templates/SHAPE_ConeIndices.h"
#include "Templates/SHAPE_SphereVertices.h"
#include "Templates/SHAPE_SphereIndices.h"

#pragma comment(lib, "Ole32.lib")

const std::vector<std::string> GroupFolders = {"Textures","Skybox","Models"};

EditorToolbar::EditorToolbar(void)
{
    this->SettingsMenuOpen      = false;
    this->ShowCameraProperties  = false;
    this->SkyboxIndexChange     = 0;
    this->ChangeSkyboxValue     = false;
    this->IsImageCompilerOpen   = false;
    this->IsTextureMenuOpen     = false;
    this->IsModelMenuOpen       = false;
    this->SystemLoaded = false;
    this->GridEnable = false;
    this->NewValues             = VALUE_POINTER;

    this->Saved = false;

    this->EnviromentViewGroup.push_back(
        {
            this->CurrentModelIndex,
            false,false,false,false,
            std::string("Sun"),
            ICON_FA_SUN,
            DATATYPE_ENVIROMENT_NOINTERACT, DATATYPE_SKY,
            glm::vec3(1.0f),glm::quat(0.0f,0.0f,0.0f,0.0f),glm::vec3(1.0f),
            glm::vec4(0.0f,0.0f,0.0f,0.0f)
        }
    );
    this->EnviromentViewGroup.push_back(
        {
            this->CurrentModelIndex,
            false,false,false,false,
            std::string("Skybox"),
            ICON_FA_CLOUD,
            DATATYPE_ENVIROMENT_NOINTERACT,DATATYPE_SUN,
            glm::vec3(1.0f),glm::quat(0.0f,0.0f,0.0f,0.0f),glm::vec3(1.0f),
            glm::vec4(0.0f,0.0f,0.0f,0.0f)
        }
    );
    this->EnviromentViewGroup.push_back(
        {
            this->CurrentModelIndex,
            false,false,false,false,
            std::string("Camera"),
            ICON_FA_CAMERA,
            DATATYPE_ENVIROMENT_NOINTERACT,DATATYPE_CAMERA,
            glm::vec3(1.0f),glm::quat(0.0f,0.0f,0.0f,0.0f),glm::vec3(1.0f),
            glm::vec4(0.0f,0.0f,0.0f,0.0f)
        }
    );
}

ImageBufferFormat EditorToolbar::ReadFiles(std::string File)
{
    std::ifstream file(File, std::ios::binary);
    if (!file.is_open()) return {};

    uint32_t w, h, c;
    uint32_t ds;
    unsigned char* Data;
;
    file.read((char*)&w, sizeof(uint32_t));
    file.read((char*)&h, sizeof(uint32_t));
    file.read((char*)&c, sizeof(uint32_t));
    file.read((char*)&ds, sizeof(uint32_t));

    std::vector<unsigned char> NewData(ds);
    file.read((char*)NewData.data(), ds);

    ImageBufferFormat buffer;
    buffer.TextureWidth = w;
    buffer.TextureHeight = h;
    buffer.TextureChannel = c;
    buffer.TextureData = NewData;
    
    file.close();

    return buffer;
}

void EditorToolbar::SetupImages()
{
    ImageBufferFormat NewFormat             = this->ReadFiles("Logo.imgbuf");
    ImageBufferFormat ThumbnailFormat       = this->ReadFiles("Thumbnail.imgbuf");
    ImageBufferFormat MissingImage          = this->ReadFiles("Camera.imgbuf");
    ImageBufferToolbar NewToolbar           = {-1,NewFormat.TextureWidth,NewFormat.TextureHeight,NewFormat.TextureChannel,NewFormat.TextureData};
    ImageBufferToolbar NewThumbnailToolbar  = { -1,ThumbnailFormat.TextureWidth,ThumbnailFormat.TextureHeight,ThumbnailFormat.TextureChannel,ThumbnailFormat.TextureData };
    ImageBufferToolbar MissingImageCamera   = { -1,MissingImage.TextureWidth,MissingImage.TextureHeight,MissingImage.TextureChannel,MissingImage.TextureData };

    this->LogoImageID = this->LoadTextureFromMemory(NewToolbar);
    this->ThumbnailImageID = this->LoadTextureFromMemory(NewThumbnailToolbar);
    this->MissingImageID = this->LoadTextureFromMemory(MissingImageCamera);
}

void EditorToolbar::PrintAllFiles(const std::string& File)
{
    memset(&this->NewDataparser, 0, sizeof(this->NewDataparser));
    if (!mz_zip_reader_init_file(&this->NewDataparser, File.c_str(), 0)) { Console::SysPrint("Failed to open Datapack file '" + File + "'.", 3); return; }

    int FileCount = (int)mz_zip_reader_get_num_files(&this->NewDataparser);
    for (int Index = 0; Index < FileCount; Index++)
    {
        mz_zip_archive_file_stat NewFileStat;
        if (!mz_zip_reader_file_stat(&this->NewDataparser, Index, &NewFileStat)) { Console::SysPrint("Failed to get file data for index" + std::to_string(Index), 1); continue; }

        std::string NewFileName(NewFileStat.m_filename);
        for (int FolderIndex = 0; FolderIndex < GroupFolders.size(); FolderIndex++)
        {
            if (NewFileName.rfind(GroupFolders[FolderIndex], 0) == 0 && !NewFileStat.m_is_directory)
            {
                unsigned int TextureID = 0;
                size_t NewSize = 0;

                void* FileDataInMemory = mz_zip_reader_extract_to_heap(&this->NewDataparser, Index, &NewSize, 0);

                if (std::string(NewFileStat.m_filename).find(".imgbuf") != std::string::npos)
                {
                    ImageBufferToolbar NewFormat;
                    std::string CurrentFileContents(static_cast<char*>(FileDataInMemory), NewSize);

                    this->LoadFromBuffer(CurrentFileContents.data(), CurrentFileContents.size(), &NewFormat);

                    if (GroupFolders[FolderIndex] == "Skybox")
                    {
                        TextureID = this->LoadTextureFromMemory(NewFormat);
                        this->SkyboxMapIndex.insert({ NewFileStat.m_filename,TextureID });
                    }
                    else
                    {
                        TextureID = this->LoadTextureFromMemory(NewFormat);
                        this->TextureMapIndex.insert({ NewFileStat.m_filename, TextureID });
                    }

                    this->NewWorkspace.TextureExtraInfo.insert({ TextureID,NewFormat });
                }
                else if (std::string(NewFileStat.m_filename).find(".gltf") != std::string::npos)
                {
                    this->ModelIndex.insert({ NewFileStat.m_filename,0 });
                }
            }
        }
    }
    mz_zip_reader_end(&this->NewDataparser);
}

void EditorToolbar::BuildObjects(int Index, DataContainer NewFormat, DataType NewDataType)
{
    //CubeInfomation NewInfomation;
    //ModelGroup NewModelGroupStruct;

    //NewInfomation.ColourCube = (
    //    (NewDataType == DATATYPE_BLOCK) ?
    //    glm::vec4(NewFormat.CurrentBlockStructure[Index].BlockColour.x, NewFormat.CurrentBlockStructure[Index].BlockColour.y, NewFormat.CurrentBlockStructure[Index].BlockColour.z, 1.0f) :
    //    glm::vec4(NewFormat.CurrentLightStructure[Index].LightColour.x, NewFormat.CurrentLightStructure[Index].LightColour.y, NewFormat.CurrentLightStructure[Index].LightColour.z, 1.0f)
    //);

    //NewInfomation.NewShapeInfomation = SHAPE_CUBE;

    //if (NewDataType == DATATYPE_BLOCK)
    //{
    //    NewInfomation.NewBoundingBox = this->NewWorkspace.EngineBoundSystem.GetBoundingBoxes(this->NewWorkspace.GroupVector3(NewFormat.CurrentBlockStructure[Index].Vertices), NewFormat.CurrentBlockStructure[Index].Position);
    //    NewInfomation.Indices = NewFormat.CurrentBlockStructure[Index].Indices;
    //    NewInfomation.Locked = NewFormat.CurrentBlockStructure[Index].PhysicsLocked;
    //    NewInfomation.PreDefinedTranslation = NewFormat.CurrentBlockStructure[Index].Position;
    //    NewInfomation.PreDefinedRotation = NewFormat.CurrentBlockStructure[Index].Rotation;
    //    NewInfomation.PreDefinedScale = NewFormat.CurrentBlockStructure[Index].Scale;

    //    NewInfomation.ColourUse = (NewFormat.CurrentBlockStructure[Index].UseBlockColour == 1.0f ? true : false);
    //    NewInfomation.TranslationMatrix = NewFormat.CurrentBlockStructure[Index].TranslationMatrix;
    //    NewInfomation.RotationMatrix = NewFormat.CurrentBlockStructure[Index].RotationMatrix;
    //    NewInfomation.ScaleMatrix = NewFormat.CurrentBlockStructure[Index].ScaleMatrix;

    //    NewInfomation.Visible = true;
    //    NewInfomation.Vertices = NewFormat.CurrentBlockStructure[Index].Vertices;
    //    NewInfomation.TextureID = (NewFormat.CurrentBlockStructure[Index].UseBlockColour == 0.0f ? NewFormat.CurrentBlockStructure[Index].TextureIndex : -1);
    //}
    //else if (NewDataType == DATATYPE_LIGHT)
    //{
    //    NewInfomation.NewBoundingBox = this->NewWorkspace.EngineBoundSystem.GetBoundingBoxes(this->NewWorkspace.GroupVector3(CubeVertices), NewFormat.CurrentLightStructure[Index].Position);
    //    NewInfomation.Indices = CubeIndices;
    //    NewInfomation.Locked = true;
    //    NewInfomation.PreDefinedTranslation = NewFormat.CurrentLightStructure[Index].Position;
    //    NewInfomation.PreDefinedRotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
    //    NewInfomation.PreDefinedScale = glm::vec3(20.0f, 20.0f, 20.0f);

    //    NewInfomation.TranslationMatrix = glm::translate(glm::mat4(1.0f), NewInfomation.PreDefinedTranslation);
    //    NewInfomation.RotationMatrix = glm::mat4_cast(NewInfomation.PreDefinedRotation);
    //    NewInfomation.ScaleMatrix = glm::scale(glm::mat4(1.0f), NewInfomation.PreDefinedScale);

    //    NewInfomation.Visible = true;
    //    NewInfomation.Vertices = CubeVertices;
    //    NewInfomation.TextureID = -1;
    //}

    //NewModelGroupStruct.Colour = glm::vec4(NewFormat.CurrentBlockStructure[Index].BlockColour.x, NewFormat.CurrentBlockStructure[Index].BlockColour.y, NewFormat.CurrentBlockStructure[Index].BlockColour.z, 1.0f);
    //NewModelGroupStruct.LabelName = std::string("NewModel (" + std::to_string(this->CurrentModelIndex) + ")");
    //NewModelGroupStruct.LabelPrefix = (NewDataType == DATATYPE_BLOCK ? ICON_FA_CUBE : ICON_FA_LIGHTBULB);
    //NewModelGroupStruct.Index = this->CurrentModelIndex;
    //NewModelGroupStruct.UseColour = NewInfomation.ColourUse;
    //NewModelGroupStruct.NewDataType = NewDataType;
    //NewModelGroupStruct.PhysicsLocked = NewFormat.CurrentBlockStructure[Index].PhysicsLocked;
    //NewModelGroupStruct.Rotate = NewFormat.CurrentBlockStructure[Index].Rotation;
    //NewModelGroupStruct.Translate = NewFormat.CurrentBlockStructure[Index].Position;
    //NewModelGroupStruct.Scale = NewFormat.CurrentBlockStructure[Index].Scale;

    //NewModelGroupStruct.Selected = false;
    //NewModelGroupStruct.Visible = true;

    //this->NewWorkspace.SetCube(NewInfomation);
    //this->CurrentModelIndex++;
    //this->NewModelGroup.push_back(NewModelGroupStruct);
}

void EditorToolbar::LoadNewWorld(std::string FileName)
{
    if (this->NewWorkspace.Loaded)
    {
        //this->NewWorkspace.CubeEntityGroup = {};
        //this->NewModelGroup = {};
        //this->CurrentModelIndex = 0;

        //FileStructureFormat NewFormat = this->NewWorkspace.NewFileCompiler.ReadFile(FileName.c_str());

        //for (int Index = 0; Index < NewFormat.CurrentBlockStructure.size(); Index++)
        //{
        //    this->BuildObjects(Index, NewFormat, DATATYPE_BLOCK);
        //}

        //for (int Index = 0; Index < NewFormat.CurrentLightStructure.size(); Index++)
        //{
        //    this->BuildObjects(Index, NewFormat, DATATYPE_LIGHT);
        //}
    }
    else
    {
        Console::SysPrint("A datapack must be loaded to load a new world.", 3);
    }
}

std::string EditorToolbar::WideToString(const std::wstring& String)
{
    if (String.empty()) return std::string();

    int SizeNeeded = WideCharToMultiByte(
        CP_UTF8, 0,
        String.c_str(), (int)String.size(),
        NULL, 0,
        NULL, NULL);

    std::string NewString(SizeNeeded, 0);

    WideCharToMultiByte(
        CP_UTF8, 0,
        String.c_str(), (int)String.size(),
        &NewString[0], SizeNeeded,
        NULL, NULL);

    return NewString;
}

std::string EditorToolbar::ShowSaveFileDialog()
{
    std::string Result;

    HRESULT InitiliseResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    IFileSaveDialog* FileSave = nullptr;
    HRESULT NewResult = CoCreateInstance(
        CLSID_FileSaveDialog,
        NULL,
        CLSCTX_ALL,
        IID_IFileSaveDialog,
        reinterpret_cast<void**>(&FileSave));


    if (SUCCEEDED(NewResult))
    {
        COMDLG_FILTERSPEC fileTypes[] =
        {
            { L"World Extended File (*.wrdx)", L"*.wrdx" },
            { L"All Files (*.*)", L"*.*" }
        };

        FileSave->SetFileTypes(2, fileTypes);
        FileSave->SetDefaultExtension(L"wrdx");

        if (SUCCEEDED(FileSave->Show(NULL)))
        {
            IShellItem* Item;
            if (SUCCEEDED(FileSave->GetResult(&Item)))
            {
                PWSTR FilePath = nullptr;

                if (SUCCEEDED(Item->GetDisplayName(SIGDN_FILESYSPATH, &FilePath)))
                {
                    std::wstring widePath(FilePath);
                    Result = this->WideToString(widePath);

                    CoTaskMemFree(FilePath);
                }

                Item->Release();
            }
        }

        FileSave->Release();
    }

    CoUninitialize();
    return Result;
}

void EditorToolbar::PromptFileExplorerView(const char* Filter, const char* FileName, StatusMode ResourceDataMode)
{
    OPENFILENAME OpenFileName;
    char File[260];
    HWND WindowHandle;
    HANDLE HandleFrame;

    ZeroMemory(&OpenFileName, sizeof(OpenFileName));
    OpenFileName.lStructSize = sizeof(OpenFileName);
    OpenFileName.lpstrFile = File;

    OpenFileName.lpstrFile[0] = '\0';
    OpenFileName.nMaxFile = sizeof(File);
    OpenFileName.lpstrFilter = Filter;
    OpenFileName.nFilterIndex = 1;
    OpenFileName.lpstrFileTitle = NULL;
    OpenFileName.nMaxFileTitle = 0;
    OpenFileName.lpstrInitialDir = NULL;
    OpenFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&OpenFileName) == TRUE)
    {
        if (std::string(File).find(FileName) != std::string::npos)
        {
            if (ResourceDataMode == STATUS_LOAD_DATAPACK)
            {
                Console::SysPrint("Opening " + std::string(File) + "...", 5);
                this->NewWorkspace.Loaded = true;
                this->PrintAllFiles(std::string(File));
                Commands::SetFromRegister("world", std::string(File));
            }
            else if (ResourceDataMode == STATUS_LOAD_WORLD)
            {
                Console::SysPrint("Opening World " + std::string(File) + "...", 5);
                this->LoadNewWorld(std::string(File));
            }
        }
    }
}

void EditorToolbar::LoadFontImages(void)
{
    ImGuiIO& NewIO = ImGui::GetIO();
    NewIO.Fonts->AddFontDefault();

    float BaseFontSize = 20.0f;
    float IconFontSize = BaseFontSize;

    static const ImWchar IconRange[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };

    ImFontConfig NewFontConfig;
    NewFontConfig.MergeMode = true;
    NewFontConfig.PixelSnapH = true;
    NewFontConfig.GlyphMinAdvanceX = IconFontSize;
    NewFontConfig.PixelSnapV = true;

    NewIO.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, IconFontSize, &NewFontConfig, IconRange);
}

void EditorToolbar::RenderToolbar(GLFWwindow* EngineWindow, Parser* EngineParser, Camera* EngineCamera, glm::mat4 Projection, glm::mat4 View, glm::vec3 Position, unsigned int Program)
{
    const float ButtonWidth = 30.0f;
    const float Spacing = 5.0f;

    if (ImGui::BeginMainMenuBar())
    {
        ImGui::Image((ImTextureRef)(this->LogoImageID), ImVec2(25.0f, 25.0f),ImVec2(0,1),ImVec2(1,0));
        ImGui::SameLine();

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "Opens the resource file")) { this->PromptFileExplorerView("Fusion DataPack\0*.fdp\0", ".fdp", STATUS_LOAD_DATAPACK); }
            ImGui::Separator();
            if (ImGui::MenuItem("Import World", "Import the world file")) { this->PromptFileExplorerView("World Extended File\0*.wrdx\0", ".wrdx", STATUS_LOAD_WORLD); }
            ImGui::Separator();
            if (ImGui::MenuItem("Export World", "Exports the world file to .wrdx"))
            {
                Console::SysPrint("----------Building application---------", 6);
                this->NewWorkspace.CompileInfomation(EngineParser,this->ShowSaveFileDialog(), EngineCamera->ReturnCameraPosition());
                this->Saved = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) { if (this->CurrentSelectedIndex != -1) { this->DeleteUIBlocks(this->CurrentSelectedIndex); this->CopyObject(this->CurrentSelectedIndex); } }
            if (ImGui::MenuItem("Copy", "CTRL+C")) { if (this->CurrentSelectedIndex != -1) { this->CopyObject(this->CurrentSelectedIndex); } }
            if (ImGui::MenuItem("Paste", "CTRL+V")) { if (this->CurrentSelectedIndex != -1) { this->PasteObject(this->CurrentSelectedIndex); } }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Texture Editor"))
        {
            if (ImGui::MenuItem((this->IsImageCompilerOpen ? "Unload Image Editor" : "Load Image Editor"), "Writes texture to global datapack"))
            {
                this->IsImageCompilerOpen = !this->IsImageCompilerOpen;
            }
            ImGui::Separator();
            if (ImGui::MenuItem((this->IsTextureMenuOpen ? "Close Texture Editor" : "Open Texture Editor"),"Edits the texture uv of geometry"))
            {
                this->IsTextureMenuOpen = !this->IsTextureMenuOpen;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Fusion Editor Settings"))
            {
                this->SettingsMenuOpen = !this->SettingsMenuOpen;
            }
            ImGui::EndMenu();
        }


        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (ButtonWidth * 3) - (Spacing * 3));
        if (ImGui::Button("_", ImVec2(ButtonWidth, 0)))
        {
            glfwIconifyWindow(EngineWindow);
        }

        ImGui::SameLine(0, Spacing);

        if (ImGui::Button((std::get<bool>(Commands::GetFromRegister("editorfullscreen")) ? "[ ]" : "[]"), ImVec2(ButtonWidth, 0)))
        {
            if (std::get<bool>(Commands::GetFromRegister("editorfullscreen")))
            {
                Commands::SetFromRegister("width", 1270);
                Commands::SetFromRegister("height", 720);
            }
            else
            {
                Commands::SetFromRegister("width", 1920);
                Commands::SetFromRegister("height", 1080);
            }

            Commands::SetFromRegister("editorfullscreen", !std::get<bool>(Commands::GetFromRegister("editorfullscreen")));
            glfwSetWindowMonitor(EngineWindow, nullptr, 100, 100, std::get<float>(Commands::GetFromRegister("width")), std::get<float>(Commands::GetFromRegister("height")), 0);
            glfwSetWindowSize(EngineWindow, std::get<float>(Commands::GetFromRegister("width")), std::get<float>(Commands::GetFromRegister("height")));
        }

        ImGui::SameLine(0, Spacing);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.1f, 0.1f, 1.00f));
        if (ImGui::Button("X", ImVec2(ButtonWidth, 0)))
        {
            if (!this->Saved)
            {
                Commands::SetFromRegister("exit", true);
            }
            else
            {
                Commands::SetFromRegister("exit", false);
                Commands::SetFromRegister("exitconfirm", true);
            }
        }
        ImGui::PopStyleColor();

       
        ImGui::EndMainMenuBar();
        if (Console::ShowExitMessage())
        {
            if (this->CurrentSelectedIndex != -1)
            {
                Console::SysPrint("----------Building application---------", 6);
                this->NewWorkspace.CompileInfomation(EngineParser,this->ShowSaveFileDialog(), EngineCamera->ReturnCameraPosition());
                this->Saved = true;
            }
            else
            {
                Console::SysPrint("Please load a datapack before exiting to save, or press 'exit' to exit.", 2);
            }
        }

        if (std::get<bool>(Commands::GetFromRegister("exitconfirm")))
        {
            for (int Index = 0; Index < this->NewWorkspace.CubeEntityGroup.size(); Index++)
            {
                this->NewWorkspace.DestroyBlocks(Index);
            }

            glfwTerminate();
            exit(0);
        }
    }
    this->NewWorkspace.RenderScene(Program, Projection, View, Position);
    this->RenderToolbarUI(EngineParser, EngineCamera);
}

void EditorToolbar::LoadFromBuffer(const char* Buffer, size_t BufferSize, ImageBufferToolbar* ImageFormatOut)
{
    const char* Pointer = Buffer;
    const char* BufferEnd = Buffer + BufferSize;
    unsigned int DataSize;

    auto ReadUnsigned = [&](unsigned int& Out) -> bool {
        if (BufferEnd - Pointer < sizeof(unsigned int)) { return false; }

        std::memcpy(&Out, Pointer, sizeof(unsigned int));
        Pointer += sizeof(unsigned int);
        return true;

        };

    if (!ReadUnsigned(ImageFormatOut->TextureWidth))    {Console::SysPrint("Corrupted buffer: missing width on image.", 3); }
    if (!ReadUnsigned(ImageFormatOut->TextureHeight))   {Console::SysPrint("Corrupted buffer: missing height on image.", 3); }
    if (!ReadUnsigned(ImageFormatOut->TextureChannel))  {Console::SysPrint("Corrupted buffer: missing channel on image.", 3); }
    if (!ReadUnsigned(DataSize)) { Console::SysPrint("Corrupted buffer: missing datasize on image.", 3); }

    if (BufferEnd - Pointer < static_cast<ptrdiff_t>(DataSize)) { Console::SysPrint("Corrupted buffer: not enough image data given declared size.", 3); }
    ImageFormatOut->TextureData.resize(DataSize);
    std::memcpy(ImageFormatOut->TextureData.data(), Pointer, DataSize);
}

unsigned int EditorToolbar::LoadTextureFromMemory(ImageBufferToolbar ImageFormatOut)
{
    unsigned int TextureID;
    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);

    unsigned int ChannelFormat = (ImageFormatOut.TextureChannel == 4) ? GL_RGBA :
        (ImageFormatOut.TextureChannel == 3) ? GL_RGB :
        (ImageFormatOut.TextureChannel == 1) ? GL_RED : GL_RGB;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, ChannelFormat, ImageFormatOut.TextureWidth, ImageFormatOut.TextureHeight, 0, ChannelFormat, GL_UNSIGNED_BYTE, &ImageFormatOut.TextureData[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    return TextureID;
}


void EditorToolbar::CopyObject(int Index)
{
    if (this->NewModelGroup.size() < Index)
    {
        Console::SysPrint("Invalid index given", 3);
        return;
    }

    this->CurrentCopyToolbar = this->NewModelGroup[Index];
    this->CurrentCopyToolbar.Index = CurrentModelIndex;

    this->NewWorkspace.CopyMainObject(Index);
}

void EditorToolbar::PasteObject(int Index)
{
    if (Index == -1) { Console::SysPrint("Nothing to paste!", 2); return; }

    this->CurrentCopyToolbar.Translate = this->NewModelGroup[Index].Translate;

    if (this->CurrentCopyToolbar.NewDataType == DATATYPE_BLOCK)
    {
        this->CurrentCopyToolbar.LabelName = std::string("NewModel (" + std::to_string(this->CurrentModelIndex) + ")");
        this->CurrentCopyToolbar.LabelPrefix = ICON_FA_CUBE;
    }
    else
    {
        this->CurrentCopyToolbar.LabelName = std::string("NewLight (" + std::to_string(this->CurrentModelIndex) + ")");
        this->CurrentCopyToolbar.LabelPrefix = ICON_FA_LIGHTBULB;
    }

    this->CurrentCopyToolbar.Visible = true;
    this->CurrentModelIndex++;
    this->NewModelGroup.push_back(this->CurrentCopyToolbar);
    this->NewWorkspace.PasteMainObject(Index);
}

void EditorToolbar::DrawDropdownMenu(std::vector<ModelGroup> Vector)
{
    if (ImGui::BeginTable("EnvHierarchy", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);

        for (int Index = 0; Index < Vector.size(); Index++)
        {
            ImGui::PushID(Index);

            const bool IsSelectable = Vector[Index].NewDataType != DATATYPE_ENVIROMENT_NOINTERACT;

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 2));

            if (this->CurrentSelectedIndex == Index && Vector[Index].NewDataType != DATATYPE_ENVIROMENT_NOINTERACT)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.65f, 0.35f, 1.00f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.22f, 0.16f, 1.00f));

            if (ImGui::Button(Vector[Index].LabelPrefix.c_str(), ImVec2(-FLT_MIN, 0)))
            {
                if (Vector[Index].LabelName == "Skybox")
                {
                    this->SunPropertiesActive = false;
                    this->CurrentSkybox = 1;
                    this->CurrentSelectedIndex = -1;
                    this->SkyboxIndexChange = 0;
                }
                else if (Vector[Index].LabelName == "Sun")
                {
                    this->SunPropertiesActive = true;
                    this->ShowCameraProperties = false;
                    this->CurrentSkybox = 0;
                    this->CurrentSelectedIndex = -1;
                }
                else if (Vector[Index].LabelName == "Camera")
                {
                    this->SunPropertiesActive = false;
                    this->ShowCameraProperties = true;
                    this->CurrentSkybox = 0;
                    this->CurrentSelectedIndex = -1;
                }
                else
                {
                    this->SunPropertiesActive = false;
                    this->ShowCameraProperties = false;
                    this->CurrentSkybox = 0;
                    if (Vector[Index].NewDataType != DATATYPE_ENVIROMENT_NOINTERACT)
                    {
                        Vector[Index].Selected = true;
                        this->CurrentSelectedIndex = Index;
                    }
                    else
                    {
                        if (Vector[Index].ExtraDataType == DATATYPE_SKY || Vector[Index].ExtraDataType == DATATYPE_SUN || Vector[Index].ExtraDataType == DATATYPE_CAMERA)
                        {
                            Vector[Index].Selected = true;
                            this->CurrentSelectedIndex = Index;
                        }
                    }
                }
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);
            ImGui::TableSetColumnIndex(1);

            ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

            if (Vector[Index].Selected) {Flags |= ImGuiTreeNodeFlags_Selected;}
       
            ImGui::Text(Vector[Index].LabelName.c_str());
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::Separator();
}

void EditorToolbar::RenderSettingsMenu(void)
{
    if (this->SettingsMenuOpen)
    {
        ImGui::Begin("Fusion Settings");

        if (ImGui::BeginTabBar("MainTabs"))
        {
            if (ImGui::BeginTabItem("Editor Variables"))
            {
                if (ImGui::BeginTable("KeyValueTable", 2))
                {
                    for (int Index = 0; Index < Commands::RegisterList.size(); Index++)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text(Commands::RegisterList[Index].Key.c_str());

                        ImGui::TableSetColumnIndex(1);
                        if (std::holds_alternative<bool>(Commands::RegisterList[Index].Value))              
                        {
                            ImGui::Checkbox(Commands::RegisterList[Index].About.c_str(), &std::get<bool>(Commands::RegisterList[Index].Value));
                        }
                        else if (std::holds_alternative<int>(Commands::RegisterList[Index].Value))          
                        {
                            ImGui::InputInt(Commands::RegisterList[Index].About.c_str(), &std::get<int>(Commands::RegisterList[Index].Value));
                        }
                        else if (std::holds_alternative<float>(Commands::RegisterList[Index].Value))        
                        {
                            ImGui::InputFloat(Commands::RegisterList[Index].About.c_str(), &std::get<float>(Commands::RegisterList[Index].Value));
                        }
                        else if (std::holds_alternative<std::string>(Commands::RegisterList[Index].Value))  
                        {
                            ImGui::InputText(Commands::RegisterList[Index].About.c_str(), &std::get<std::string>(Commands::RegisterList[Index].Value));
                        }
                    }
                    ImGui::EndTable();
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("About"))
            {
                ImGui::Image((ImTextureRef)(this->ThumbnailImageID), ImVec2(550.0f, 150.0f));
                ImGui::Separator();
                ImGui::NewLine();
 

                ImGui::Text("Build Version: 0.20\n");
                ImGui::Text("Release Version: 0.25\n");
                #ifdef SET_AS_ALPHA
                    ImGui::Text("This product is in early alpha!");
                #endif
                ImGui::NewLine();
                ImGui::Text("Dependancies: \nImGui\nImGuizmo\nNlohmann json\nGlad\nminiz\nGLM\nGLFW\nImGuizmo\nSTB\n(Depdancies Licences Found Within Source/ReadMe)");

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}

void EditorToolbar::RenderWorldView(void)
{
    std::vector<std::string> DuplicateNames = {};
    ImGui::Begin("World Explorer");

    if (this->CurrentSelectedIndex != -1)
    {
        this->NewWorkspace.SetColour(this->CurrentSelectedIndex, this->NewModelGroup[this->CurrentSelectedIndex].UseColour, this->NewModelGroup[this->CurrentSelectedIndex].Colour);
        this->NewWorkspace.SetVisible(this->CurrentSelectedIndex, this->NewModelGroup[this->CurrentSelectedIndex].Visible);
        this->NewWorkspace.SetPosition(this->CurrentSelectedIndex, this->NewModelGroup[this->CurrentSelectedIndex].Translate);
        this->NewWorkspace.SetRotation(this->CurrentSelectedIndex, this->NewModelGroup[this->CurrentSelectedIndex].Rotate);
        this->NewWorkspace.SetScale(this->CurrentSelectedIndex, this->NewModelGroup[this->CurrentSelectedIndex].Scale);
        this->NewWorkspace.SetLockedPhysics(this->CurrentSelectedIndex, this->NewModelGroup[this->CurrentSelectedIndex].PhysicsLocked);
    }

    this->DrawDropdownMenu(this->EnviromentViewGroup);
    this->DrawDropdownMenu(this->NewModelGroup);

    ImGui::End();
}

void EditorToolbar::RenderImageCompiler(void)
{
    static glm::vec2 OldUV      = {0.0f,0.0f};
    static glm::vec2 OldScale   = {0.0f,0.0f};

    static glm::vec2 TextureUV      = {0.0f,0.0f};
    static glm::vec2 TextureScale   = {0.0f,0.0f};

    static glm::vec2 OriginalUV     = {0.0f,0.0f};
    static glm::vec2 OriginalScale  = {0.0f,0.0f};
    static bool OriginalSet         = false;

    if (this->IsImageCompilerOpen) 
    {
        ImGui::Begin("ImageCompiler");
        ImGui::Image((ImTextureRef)(this->MissingImageID), ImVec2(300.0f, 300.0f));

        ImGui::Text("Image Width: 0 px");
        ImGui::Separator();
        ImGui::Text("Image Height: 0 px");
        ImGui::Separator();
        ImGui::Text("Image Channel: 0");
        ImGui::Separator();

        if (ImGui::Button("Load Image")) {this->PromptFileExplorerView("JPG\0*.jpg\0", ".jpg", STATUS_LOAD_IMAGE);}
        ImGui::SameLine();

        if (ImGui::Button("Write Image"))
        {
            //TODO
        }

        ImGui::End();
    }

    if (this->IsTextureMenuOpen)
    {
        static unsigned int ViewTextureIndex;
        if (this->CurrentSelectedIndex != -1)
        {
            if (this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].ColourUse)
            {
                ViewTextureIndex = this->MissingImageID;
            }
            else
            {
                ViewTextureIndex = this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].TextureID;
            }
        }
        else
        {
            ViewTextureIndex = this->MissingImageID;
            TextureUV       = glm::vec2(0.0f, 0.0f);
            OldUV           = glm::vec2(0.0f, 0.0f);
            TextureScale    = glm::vec2(0.0f, 0.0f);
            OldUV           = glm::vec2(0.0f, 0.0f);
        }

        if (!OriginalSet && this->CurrentSelectedIndex != -1)
        {
            for (int Index = 0; Index < this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].Vertices.size(); Index += 8)
            {
                this->TextureOffset.push_back(
                    glm::vec2(this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].Vertices[Index + 6],
                        this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].Vertices[Index + 7]
                    )
                );
            }
            OriginalSet = true;
        }

        ImGui::Begin("Texture Editor");
        ImGui::Image((ImTextureRef)(ViewTextureIndex), ImVec2(300.0f, 300.0f));
        ImGui::Text("Texture Offset");

        static float OffsetStep = 0.1f;
        ImGui::DragFloat("Offset Step", &OffsetStep, 0.01f, 0.01f, 10.0f);

        ImGui::PushItemWidth(80);

        ImGui::Text("U"); ImGui::SameLine();
        ImGui::DragFloat("##offsetU", &TextureUV.x, 0.01f, -100.0f, 100.0f);
        ImGui::SameLine();
        if (ImGui::Button("+##offsetU")) TextureUV.x += OffsetStep;
        ImGui::SameLine();
        if (ImGui::Button("-##offsetU")) TextureUV.x -= OffsetStep;

        ImGui::Text("V"); ImGui::SameLine();
        ImGui::DragFloat("##offsetV", &TextureUV.y, 0.01f, -100.0f, 100.0f);
        ImGui::SameLine();
        if (ImGui::Button("+##offsetV")) TextureUV.y += OffsetStep;
        ImGui::SameLine();
        if (ImGui::Button("-##offsetV")) TextureUV.y -= OffsetStep;

        ImGui::PopItemWidth();

        if (TextureUV != OldUV)
        {
            this->EditTextureObject(0, TextureUV);
            OldUV = TextureUV;
        }
        ImGui::Separator();
        ImGui::Text("Texture Scale");

        ImGui::PushItemWidth(80);

        ImGui::Text("U"); ImGui::SameLine();

        ImGui::DragFloat("##scaleU", &TextureScale.x, 0.01f, 0.01f, 100.0f);
        ImGui::SameLine();

        if (ImGui::Button("+##U")) TextureScale.x += 0.1f;
        ImGui::SameLine();

        if (ImGui::Button("-##U")) TextureScale.x -= 0.1f;


        ImGui::Text("V"); ImGui::SameLine();

        ImGui::DragFloat("##scaleV", &TextureScale.y, 0.01f, 0.01f, 100.0f);
        ImGui::SameLine();

        if (ImGui::Button("+##V")) TextureScale.y += 0.1f;
        ImGui::SameLine();

        if (ImGui::Button("-##V")) TextureScale.y -= 0.1f;


        if (ImGui::Button("Uniform"))
        {
            TextureScale.y = TextureScale.x;
        }

        ImGui::SameLine();

        if (ImGui::Button("Half"))
        {
            TextureScale *= 0.5f;
        }

        ImGui::SameLine();

        if (ImGui::Button("Double"))
        {
            TextureScale *= 2.0f;
        }

        ImGui::PopItemWidth();

        if (TextureScale != OldScale)
        {
            glm::vec2 Delta = glm::vec2(
                OldScale.x == 0 ? 1.0f : TextureScale.x / OldScale.x,
                OldScale.y == 0 ? 1.0f : TextureScale.y / OldScale.y
            );

            this->EditTextureObject(2,Delta);
            OldScale = TextureScale;
        }

        if (ImGui::Button("Flip U")) TextureScale.x *= -1.0f;
        ImGui::SameLine();
        if (ImGui::Button("Flip V")) TextureScale.y *= -1.0f;

        ImGui::Separator();
        ImGui::Text("Texture Rotation");

        static float TextureRotation = 0.0f;

        ImGui::DragFloat("Rotation", &TextureRotation, 1.0f, -360.0f, 360.0f);

        if (ImGui::Button("Rotate +90"))
        {
            TextureRotation += 90.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Rotate -90"))
        {
            TextureRotation -= 90.0f;
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            TextureRotation = 0.0f;
        }

        ImGui::Separator();
        if (ImGui::Button("Set Default"))
        {
            this->EditTextureObject(1, OriginalUV);

            TextureUV = {0.0f,0.0f};
            TextureScale = { 0.0f,0.0f };

            OldScale = TextureScale;
            OldUV = TextureUV;
        }

        ImGui::End();
    }
}

void EditorToolbar::EditTextureObject(int ActionObject, glm::vec2 TextureData)
{
    if (this->CurrentSelectedIndex == -1) return;

    auto& mesh = this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex];
    auto& Vertices = mesh.Vertices;

    if (ActionObject == 0) 
    {
        for (int i = 0; i < Vertices.size(); i += 8)
        {
            Vertices[i + 6] += TextureData.x;
            Vertices[i + 7] += TextureData.y;
        }
    }
    else if (ActionObject == 1) 
    {
        for (int i = 0; i < Vertices.size(); i += 8)
        {
            int texIndex = i / 8;
            if (texIndex < this->TextureOffset.size())
            {
                Vertices[i + 6] = this->TextureOffset[texIndex].x;
                Vertices[i + 7] = this->TextureOffset[texIndex].y;
            }
        }
    }
    else if (ActionObject == 2) 
    {
        glm::vec2 Center(0.0f);
        int count = 0;

        for (int i = 0; i < Vertices.size(); i += 8)
        {
            Center.x += Vertices[i + 6];
            Center.y += Vertices[i + 7];
            count++;
        }

        Center /= (float)count;

        for (int i = 0; i < Vertices.size(); i += 8)
        {
            float& U = Vertices[i + 6];
            float& V = Vertices[i + 7];

            U = Center.x + (U - Center.x) * TextureData.x;
            V = Center.y + (V - Center.y) * TextureData.y;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBuffer);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        Vertices.size() * sizeof(float),
        Vertices.data()
    );
}

std::vector<float> EditorToolbar::PositionToFloat(const std::vector<ModelVertex>& Vertices)
{
    std::vector<float> FloatVertices;
    for (int Index = 0; Index < Vertices.size(); Index++)
    {
        FloatVertices.push_back(Vertices[Index].Positions.x);
        FloatVertices.push_back(Vertices[Index].Positions.y);
        FloatVertices.push_back(Vertices[Index].Positions.z);
        FloatVertices.push_back(Vertices[Index].Normals.x);
        FloatVertices.push_back(Vertices[Index].Normals.y);
        FloatVertices.push_back(Vertices[Index].Normals.z);
        FloatVertices.push_back(Vertices[Index].TextureUV.x);
        FloatVertices.push_back(Vertices[Index].TextureUV.y);
    }
    return FloatVertices;
}

void EditorToolbar::RenderTextureViewer(Camera* EngineCamera,Parser* EngineParser)
{
    ImGui::Begin("Asset Viewer");

    int CurrentIndex                        = 0;
    static std::string FolderSelectedName   = "Textures";

    for (int Index = 0; Index < GroupFolders.size(); Index++)
    {
        if (ImGui::Button(GroupFolders[Index].c_str())) {FolderSelectedName = GroupFolders[Index];}
        ImGui::SameLine();
    }

    ImGui::SameLine();

    ImGui::PushItemWidth(150);
    ImGui::InputText("Search",&this->SearchSystem);
    ImGui::PopItemWidth();

    ImGui::NewLine();

    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        int Count               = 0;
        float ThumbnailSize     = 128.0f;
        float Padding           = 16.0f;
        float CellSize          = ThumbnailSize + Padding;

        float AvailX    = ImGui::GetContentRegionAvail().x;
        int Columns     = (int)(AvailX / CellSize);
        
        std::string NewString;
        static std::map<std::string, unsigned int> ObjectMapIndex = {};

        if (FolderSelectedName != "")
        {
            if (FolderSelectedName == GroupFolders[0])
            {
                ObjectMapIndex = this->TextureMapIndex;
            }
            else if (FolderSelectedName == GroupFolders[1])
            {
                ObjectMapIndex = this->SkyboxMapIndex;
            }
            else if (FolderSelectedName == GroupFolders[2])
            {
                ObjectMapIndex = this->ModelIndex;
            }
        }

        if (Columns < 1) Columns = 1;
        for (std::map<std::string, unsigned int>::iterator Iterator = ObjectMapIndex.begin(); Iterator != ObjectMapIndex.end(); Iterator++)
        {
            NewString = "";
            ImGui::BeginGroup();

            if (Iterator->first.size() > 15)    {NewString = Iterator->first.substr(0, 15) + "...";}
            else                                {NewString = Iterator->first;}

            if (ImGui::ImageButton(Iterator->first.c_str(), (ImTextureID)(intptr_t)(Iterator->second), ImVec2(ThumbnailSize, ThumbnailSize), ImVec2(0, 1), ImVec2(1, 0)))
            {
                if (FolderSelectedName == GroupFolders[2])
                {
                    glm::vec3 Translate = EngineCamera->ReturnCameraPosition();
                    glm::quat Rotate    = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
                    glm::vec3 Scale     = glm::vec3(1.0f, 1.0f, 1.0f);

                    Mesh NewMesh;
                    NewMesh.LoadMeshUsingGLTF(EngineParser,Iterator->first);
                    if (NewMesh.GetMeshVertices().size() != 0 && NewMesh.GetMeshIndices().size() != 0)
                    {
                        this->SpawnObject(SHAPE_MESH, "MeshObject", ICON_FA_CUBE, DATATYPE_BLOCK, Translate, Rotate, Scale, this->PositionToFloat(NewMesh.GetMeshVertices()), NewMesh.GetMeshIndices());
                        this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].FileName = std::regex_replace(Iterator->first, std::regex("\\Models/"), "");
                    }
                    else
                    {
                        Console::SysPrint("Failed to load model: "+Iterator->first, 2);
                    }
                    this->MeshGroup.push_back(NewMesh);
                }
                else if (FolderSelectedName == GroupFolders[1] && this->ChangeSkyboxValue)
                {
                    this->NewWorkspace.SkyboxValues[this->SkyboxIndexChange] = Iterator->first;
                    Commands::SetFromRegister("changeskybox", true);
                    this->ChangeSkyboxValue = false;
                }
                else
                {
                    if (this->CurrentSelectedIndex != -1)
                    {
                        this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].TextureID = Iterator->second;
                        this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].TextureName = Iterator->first;
                        Console::SysPrint(("Using texture: " + Iterator->first).c_str(), 2);
                    }
                }
            }

            ImGui::TextWrapped("%s", NewString.c_str());
            ImGui::EndGroup();

            if (++Count % Columns != 0) { ImGui::SameLine(); }
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

void EditorToolbar::RenderSkyboxProperties(void)
{
    if (this->CurrentSkybox == 1 && this->SystemLoaded)
    {
        ImGui::SeparatorText("Skybox Properties");
        for (int Index = 0; Index < this->NewWorkspace.SkyboxValues.size(); Index++)
        {
            ImGui::NewLine();
            ImGui::Image((ImTextureID)(this->SkyboxMapIndex.find(this->NewWorkspace.SkyboxValues[Index])->second), ImVec2(50, 50));
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, (this->ChangeSkyboxValue && this->SkyboxIndexChange == Index) ? ImVec4(0.10f, 0.65f, 0.35f, 1.00f) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
            
            ImGui::BeginGroup();
            {
                ImGui::Text(this->NewWorkspace.SkyboxValues[Index].c_str());
                if (ImGui::Button(("Change Image" + this->Dimensions[Index]).c_str()))
                {
                    if (!this->ChangeSkyboxValue)
                    {
                        Console::SysPrint("Skybox Selection active! Select your skybox from textures.", 1);
                        this->SkyboxIndexChange = Index;
                        this->ChangeSkyboxValue = true;
                    }
                }
            }
            ImGui::EndGroup();
            ImGui::PopStyleColor();
        }
    }
}

void EditorToolbar::RenderCameraProperties(void)
{
    if (this->ShowCameraProperties && this->SystemLoaded)
    {
        ImGui::SeparatorText("Camera Properties");
    }
}

void EditorToolbar::RenderSunProperties(void)
{
    if (this->SunPropertiesActive && this->SystemLoaded)
    {
        ImGui::SeparatorText("Sun Properties");
    }
}

void EditorToolbar::RenderGeometryProperties(void)
{
    if (this->CurrentSelectedIndex != -1 && this->SystemLoaded)
    {
        ImGui::SeparatorText("Model Properties");

        this->SelectedMaterial = this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].MaterialIndex;
        this->LastKnownName = this->NewModelGroup[this->CurrentSelectedIndex].LabelName;

        ImGui::Text("Name:");
        ImGui::InputText("       ",&this->NewModelGroup[this->CurrentSelectedIndex].LabelName);

        if (this->NewModelGroup[this->CurrentSelectedIndex].LabelName.size() == 0)
        {
            this->NewModelGroup[this->CurrentSelectedIndex].LabelName = this->LastKnownName;
        }

        ImGui::Text("DataType: %s", (this->NewModelGroup[this->CurrentSelectedIndex].NewDataType == DATATYPE_BLOCK ? "Block" : (this->NewModelGroup[this->CurrentSelectedIndex].NewDataType == DATATYPE_LIGHT ? "Light" : "?")));
        ImGui::Separator();

        if (this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].NewShapeInfomation != SHAPE_MESH)
        {
            ImGui::Text("Shape:");
            static const char* Items[] = { "Cube","Sphere","Cone" };
            if (ImGui::Combo(" ", &this->SelectedItems, Items, IM_ARRAYSIZE(Items)))
            {
                Console::SysPrint(std::to_string(this->SelectedItems), 2);
            }
            ImGui::Separator();
        }

        ImGui::Text("Material:");
        static const char* MatItems[] = { "None", "Fluid","Metal" };
        if (ImGui::Combo("   ", &this->SelectedMaterial, MatItems, IM_ARRAYSIZE(MatItems)))
        {
            this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].MaterialIndex = this->SelectedMaterial;
            Console::SysPrint("Selected Material Index " + std::to_string(this->SelectedMaterial), 2);
        }
        ImGui::Separator();

        ImGui::Text("Transform");
        ImGui::DragFloat3("Position", &this->NewModelGroup[this->CurrentSelectedIndex].Translate.x, 0.1f);
        ImGui::DragFloat4("Rotation", &this->NewModelGroup[this->CurrentSelectedIndex].Rotate.x, 0.1f);
        ImGui::DragFloat3("Scale", &this->NewModelGroup[this->CurrentSelectedIndex].Scale.x, 0.1f);
        ImGui::Separator();

        ImGui::Checkbox("Physics Locked", &this->NewModelGroup[this->CurrentSelectedIndex].PhysicsLocked);
        ImGui::Separator();

        ImGui::Checkbox("Visible", &this->NewModelGroup[this->CurrentSelectedIndex].Visible);
        ImGui::Separator();

        ImGui::Checkbox("Use Colour", &this->NewModelGroup[this->CurrentSelectedIndex].UseColour);
        ImGui::ColorEdit4("Colour", (float*)&this->NewModelGroup[this->CurrentSelectedIndex].Colour);

        ImGui::Separator();
        ImGui::Text("OpenGL Render Values");
        ImGui::Text("Vertex Array Index: %i", this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].VertexArray);
        ImGui::Text("Vertex Buffer Index: %i", this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].VertexBuffer);
        ImGui::Text("Element Buffer Index: %i", this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].ElementBuffer);
        ImGui::Text("Mesh Texture Index: %i", (this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].TextureID));
        ImGui::Separator();
        

        if (this->NewModelGroup[this->CurrentSelectedIndex].NewDataType != DATATYPE_LIGHT)
        {
            ImGui::Text("Mesh Texture");
            ImGui::Text(("Diffuse Texture: " + (this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].ColourUse ? "None" : this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].TextureName)).c_str());
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Image((ImTextureID)(intptr_t)(this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].ColourUse || this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].TextureID == -1 ? this->MissingImageID : this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].TextureID), ImVec2(100, 100));
                ImGui::EndTooltip();
            }


            ImGui::Text(("Specular Texture: " + (this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].ColourUse ? "None" : this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].SpecularName)).c_str());
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Image((ImTextureID)(intptr_t)(this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].ColourUse || this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].SpecularID == -1 ? this->MissingImageID : this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].SpecularID), ImVec2(100, 100));
                ImGui::EndTooltip();
            }

            ImGui::Text(("Normal Texture: "+ (this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].ColourUse ? "None" : this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].NormalName)).c_str());
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Image((ImTextureID)(intptr_t)(this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].ColourUse || this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].NormalID == -1 ? this->MissingImageID : this->NewWorkspace.CubeEntityGroup[this->CurrentSelectedIndex].NormalID), ImVec2(100, 100));
                ImGui::EndTooltip();
            }

            ImGui::Separator();
        }
    }
}

void EditorToolbar::RenderMeshProperties(void)
{
    ImGui::Begin("Properties");

    if (this->CurrentSelectedIndex != -1)       {this->RenderGeometryProperties();}
    else
    {
        if (this->CurrentSkybox == 1)           {this->RenderSkyboxProperties();}
        else if (this->ShowCameraProperties)    {this->RenderCameraProperties();}
        else if (this->SunPropertiesActive)     {this->RenderSunProperties();}
    }
    ImGui::End();

    if (this->IsAddMenuOpen)                    {this->CreateAddMenu();}
    this->RenderSettingsMenu();
}

void EditorToolbar::SpawnObject(ShapeInfomation ShapeInfo, std::string Name, std::string Prefix, DataType Object, glm::vec3 Position, glm::quat Rotation, glm::vec3 Scale,std::vector<float> Vertices,std::vector<unsigned int> Indices)
{
    if (!this->SystemLoaded) { Console::SysPrint("Please load a datapack before spawning objects", 3);  return; }
    this->Saved = false;

    unsigned int NewTextureID   = -1;
    std::string NewTextureName  = "";

    if (ShapeInfo == SHAPE_PLAYER)
    {
        NewTextureID    = this->TextureMapIndex.find("Textures/Spawn.imgbuf")->second;
        NewTextureName  = "Textures/Spawn.imgbuf";
    }

    glm::vec3 NewPosition = glm::vec3(0.0f);
   
    NewPosition = glm::vec3(
        (round(Position.x + Console::IncrementValue[0] / 2) / Console::IncrementValue[0]),
        (round(Position.y + Console::IncrementValue[1] / 2) / Console::IncrementValue[1]),
        (round(Position.z + Console::IncrementValue[2] / 2) / Console::IncrementValue[2])
    );

    this->NewWorkspace.CreateCube(Vertices,Indices,NewPosition, Rotation, Scale, ShapeInfo, NewTextureID, NewTextureName);
    this->NewModelGroup.push_back(
        {
            this->CurrentModelIndex,
            false,true,(ShapeInfo == SHAPE_LIGHTING ? true : false),true,
            Name + (this->CurrentModelIndex == 0 ? "" : " (" + std::to_string(this->CurrentModelIndex) + ")"),
            Prefix,
            Object,DATATYPE_NULL,
            Position,Rotation,Scale,
            (ShapeInfo == SHAPE_LIGHTING ? glm::vec4(1.0f,1.0f,1.0f,1.0f) : glm::vec4(0.0f,0.0f,0.0f,1.0f))
        }
    );

    this->CurrentModelIndex++;
    this->CurrentSelectedIndex = this->CurrentModelIndex - 1;
}

void EditorToolbar::RenderMainToolbar(Parser* EngineParser, Camera* EngineCamera)
{
    ImGui::SetCursorPos(ImVec2(8.0f, 5.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7, 18));

    auto ModeButton = [&](const char* Icon, TooltipValues Mode, const ImVec4& ActiveColor)
        {
            bool IsActive = (this->NewValues == Mode);
            ImGui::PushStyleColor(ImGuiCol_Button, IsActive ? ActiveColor : ImGui::GetStyleColorVec4(ImGuiCol_Button));

            if (ImGui::Button(Icon, ImVec2(40.0f, 40.0f))) {this->NewValues = Mode;}
            ImGui::PopStyleColor();
        };

    ModeButton(ICON_FA_MOUSE_POINTER, VALUE_POINTER, ImVec4(0.10f, 0.65f, 0.35f, 1.00f));
    ImGui::SameLine();
    ModeButton(ICON_FA_ARROWS_ALT, VALUE_TRANSLATE, ImVec4(0.10f, 0.65f, 0.35f, 1.00f));
    ImGui::SameLine();
    ModeButton(ICON_FA_SYNC, VALUE_ROTATE, ImVec4(0.10f, 0.65f, 0.35f, 1.00f));
    ImGui::SameLine();
    ModeButton(ICON_FA_EXPAND, VALUE_SCALE, ImVec4(0.10f, 0.65f, 0.35f, 1.00f));

    ImGui::SetCursorPos(ImVec2(200.0f, 5.0f));
    if (ImGui::Button(ICON_FA_CUBE, ImVec2(40.0f, 40.0f)))
    {
        Console::SysPrint("Added cube to world.", 2);
        glm::vec3 Translate = EngineCamera->ReturnCameraPosition();
        glm::quat Rotate = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec3 Scale = glm::vec3(20.0f, 20.0f, 20.0f);

        this->SpawnObject(SHAPE_CUBE, "NewModel", ICON_FA_CUBE, DATATYPE_BLOCK, Translate, Rotate, Scale, CubeVertices,CubeIndices);
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Spawn Cube");
        ImGui::EndTooltip();
    }

    ImGui::SetCursorPos(ImVec2(247.0f, 5.0f));
    if (ImGui::Button(ICON_FA_LIGHTBULB, ImVec2(40.0f, 40.0f)))
    {
        glm::vec3 Translate = EngineCamera->ReturnCameraPosition();
        glm::quat Rotate = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 Scale = glm::vec3(5.0f, 5.0f, 5.0f);

        Console::SysPrint("Added light to world", 1);
        this->SpawnObject(SHAPE_LIGHTING, "NewLight", ICON_FA_LIGHTBULB, DATATYPE_LIGHT, Translate, Rotate, Scale,CubeVertices,CubeIndices);
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Spawn Light");
        ImGui::EndTooltip();
    }

    ImGui::SetCursorPos(ImVec2(294.0f, 5.0f));
    if (ImGui::Button(ICON_FA_CODE, ImVec2(40.0f, 40.0f)))
    {
        if (this->SystemLoaded)
        {
            Console::SysPrint("----------Building application---------", 6);
            this->NewWorkspace.CompileInfomation(EngineParser,this->ShowSaveFileDialog(), EngineCamera->ReturnCameraPosition());
        }
        else
        {
            Console::SysPrint("Fusion cannot compile objects without the datapack being loaded", 3);
        }
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Compile Scene");
        ImGui::EndTooltip();
    }

    ImGui::SetCursorPos(ImVec2(341.0f, 5.0f));
    if (ImGui::Button(ICON_FA_TRASH, ImVec2(40.0f, 40.0f)))
    {
        if (this->CurrentSelectedIndex == -1) { Console::SysPrint("An object must be selected!", 2); }
        else { this->DeleteUIBlocks(this->CurrentSelectedIndex); }
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Delete entity");
        ImGui::EndTooltip();
    }

    ImGui::SetCursorPos(ImVec2(388.0f, 5.0f));
    if (ImGui::Button(ICON_FA_USER, ImVec2(40.0f, 40.0f)))
    {
        glm::vec3 Translate = EngineCamera->ReturnCameraPosition();
        glm::quat Rotate    = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec3 Scale     = glm::vec3(5.0f, 20.0f, 5.0f);

        Console::SysPrint("Added spawn to world.", 1);
        this->SpawnObject(SHAPE_PLAYER, "PlayerSpawn", ICON_FA_USER, DATATYPE_PLAYER, Translate, Rotate, Scale, CubeVertices,CubeIndices);
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Spawn PlayerSpawn");
        ImGui::EndTooltip();
    }

    ImGui::SetCursorPos(ImVec2(435.0f, 5.0f));
    if (ImGui::Button(ICON_FA_PLUS, ImVec2(40.0f, 40.0f)))
    {
        this->IsAddMenuOpen = !this->IsAddMenuOpen;
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text((this->IsAddMenuOpen ? "Close Compontent Menu" : "Open Compontent Menu"));
        ImGui::EndTooltip();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    ImGui::PushItemWidth(100);
    ImGui::SetCursorPos(ImVec2(485.0f, 5.0f));

    ImGui::InputFloat("     ", &Console::IncrementValue[0],0.5f,100);
    ImGui::SetCursorPos(ImVec2(495.0f, 30.0f));
    ImGui::Text("Increment X");

    ImGui::SetCursorPos(ImVec2(595.0f, 5.0f));
    ImGui::InputFloat("      ", &Console::IncrementValue[1], 0.5f, 100);
    ImGui::SetCursorPos(ImVec2(605.0f, 30.0f));
    ImGui::Text("Increment Y");

    ImGui::SetCursorPos(ImVec2(705.0f, 5.0f));
    ImGui::InputFloat("       ", &Console::IncrementValue[2], 0.5f, 100);
    ImGui::SetCursorPos(ImVec2(710.0f, 30.0f));
    ImGui::Text("Increment Z");

    ImGui::SetCursorPos(ImVec2(825.0f, 5.0f));
    ImGui::Checkbox("Toggle Vertex Editor",&Console::EnableVertex);

    ImGui::SetCursorPos(ImVec2(825.0f, 28.0f));
    ImGui::Checkbox("Toggle Collisions", &Console::EnableCollision);

    ImGui::SetCursorPos(ImVec2(1000.0f, 5.0f));
    ImGui::Checkbox("Toggle Grid Edit", &this->GridEnable);

    ImGui::PopItemWidth();

}

void EditorToolbar::CreateAddMenu(void)
{
    std::vector<std::string> Options = { "Add Mesh","Add Cube","Add Lights","Add Player"};
    std::vector<std::string> Icons = { ICON_FA_CUBE,ICON_FA_CUBE,ICON_FA_LIGHTBULB,ICON_FA_MAGIC };

    ImGui::Begin("Component Menu");
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar))
    {
        int Count = 0;
        float ThumbnailSize = 128.0f;
        float Padding = 16.0f;
        float CellSize = ThumbnailSize + Padding;

        float AvailX = ImGui::GetContentRegionAvail().x;
        int Columns = (int)(AvailX / CellSize);
        std::string NewString;

        if (Columns < 1) Columns = 1;

        for (int Index = 0; Index < Options.size(); Index++)
        {
            std::string Label = Icons[Index] + " " + Options[Index];
            if (ImGui::Button(Label.c_str(), ImVec2(120, 30)))
            {
                
            }
            if (++Count % Columns != 0) { ImGui::SameLine(); }
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

void EditorToolbar::DeleteUIBlocks(int Index)
{
    this->NewWorkspace.DestroyBlocks(Index);
    this->NewModelGroup.erase(this->NewModelGroup.begin() + Index);
    this->CurrentSelectedIndex = -1;
    this->CurrentModelIndex--;
}

void EditorToolbar::RenderToolbarUI(Parser* EngineParser, Camera* EngineCamera)
{
    ImGuiViewport* Viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(Viewport->Pos.x, Viewport->Pos.y + Console::MenuBarHeight + 20));
    ImGui::SetNextWindowSize(ImVec2(Viewport->Size.x, 50));
    ImGui::SetNextWindowViewport(Viewport->ID);

    ImGuiWindowFlags Window_flags = 0
        | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoSavedSettings
        ;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Toolbar", NULL, Window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    this->RenderMainToolbar(EngineParser, EngineCamera);
    this->RenderImageCompiler();

    Viewport->WorkSize = ImVec2(Viewport->WorkSize.x, Viewport->WorkSize.y - Console::MenuBarHeight - 49);
    Viewport->WorkPos = ImVec2(Viewport->WorkPos.x, Viewport->WorkPos.y + Console::MenuBarHeight + 49);

    ImGui::End();
}