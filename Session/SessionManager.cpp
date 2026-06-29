#include "SessionManager.h"

SessionManager::SessionManager(void) 
{
    this->FramesPerSecond       = 0;
    this->DeltaTime             = 0.0;
    this->PointerPosition       = glm::vec3(0.0f);
    this->FrameBufferIdentifer  = -1;
    this->TextureIdentifer      = -1;
    this->RenderIdentifer       = -1;
    this->CurrentWorldFile      = "";
    this->FirstTimeInitilized   = true;
    this->ViewportWidth         = 0;
    this->ViewportHeight        = 0;

    this->CurrentTime           = 0.0;
    this->PreviousTime          = 0.0;
    this->EngineDelayInput      = 0.0;
    this->MousePositionX        = 0.0;
    this->MousePositionY        = 0.0;
    this->Fullscreen            = false;
    this->UseVSync              = true;
}

GLFWwindow* SessionManager::ReturnEngineWindow(void)        {return this->EngineWindow;}
glm::vec2 SessionManager::GetWindowResolution(void)         {return glm::vec2(this->Width,this->Height);}
glm::vec2 SessionManager::GetCurrentMousePosition(void)     {return glm::vec2(this->MousePositionX,this->MousePositionY);}

void SessionManager::CreateSession(void)
{
    if (!glfwInit()) {MessageBox(NULL,TEXT("Failed to load GLFW windows."),TEXT("Fusion Engine"),MB_ICONERROR|MB_OK); glfwTerminate();}
    
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES,GL_MSAA_SAMPLES);
    
    this->Width         = std::get<float>(Commands::GetFromRegister("width"));
    this->Height        = std::get<float>(Commands::GetFromRegister("height"));
    this->Fullscreen    = std::get<bool>(Commands::GetFromRegister("usefullscreen"));
    this->UseVSync      = std::get<bool>(Commands::GetFromRegister("vsyncactive"));

    this->EngineWindow  = glfwCreateWindow(this->Width,this->Height,"Fusion Editor",NULL,NULL);
    
    glfwMakeContextCurrent(this->EngineWindow);
    gladLoadGL();
    glViewport(0,0,this->Width,this->Height);
    glfwSwapInterval(1);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    glfwSetWindowUserPointer(this->EngineWindow,this);
	this->FirstTimeInitilized = true;
	this->CurrentWorldFile = FUSION_WORLD_EMPTY;

    if (this->EngineWindow == NULL)                             {glfwTerminate();}
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))    {glfwTerminate();}
}

void SessionManager::CalculateDeltaTime(void)
{
    this->CurrentTime   = glfwGetTime();
    this->DeltaTime     = this->CurrentTime-this->PreviousTime;
    this->Counter++;

    if (this->DeltaTime >= 1.0f/60.0f)
    {this->FramesPerSecond = (1.0f/this->DeltaTime) * this->Counter; this->Counter = 0; this->PreviousTime = this->CurrentTime;}
}

void SessionManager::UpdateWorld(const std::string& WorldDirectory)
{
    Console::SysPrint("Started session!",5);
    Commands::SetFromRegister("world",WorldDirectory);

	this->CurrentWorldFile = WorldDirectory;
    this->EngineParser.FetchFromDatapack(WorldDirectory);
}

void SessionManager::UpdateEngineValue(void)
{
    this->Near                      = std::get<float>(Commands::GetFromRegister("near"));
    this->Far                       = std::get<float>(Commands::GetFromRegister("far"));
    this->FieldOfView               = std::get<float>(Commands::GetFromRegister("fov"));
    Console::IsDebugReferenceOpen   = std::get<bool>(Commands::GetFromRegister("debugreference"));
}

void SessionManager::CreateFrameBuffer(void)
{
    glGenFramebuffers(1,&this->FrameBufferIdentifer);
    glBindFramebuffer(GL_FRAMEBUFFER,this->FrameBufferIdentifer);

    glGenTextures(1,&this->TextureIdentifer);
    glBindTexture(GL_TEXTURE_2D,this->TextureIdentifer);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,this->Width,this->Height,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,this->TextureIdentifer,0);

    glGenRenderbuffers(1,&this->RenderIdentifer);
    glBindRenderbuffer(GL_RENDERBUFFER,this->RenderIdentifer);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,this->Width,this->Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,this->RenderIdentifer);

    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glBindTexture(GL_TEXTURE_2D,0);
    glBindRenderbuffer(GL_RENDERBUFFER,0);
}

void SessionManager::BindFrameBuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER,this->FrameBufferIdentifer);
}

void SessionManager::UnbindFrameBuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void SessionManager::RescaleFrameBuffer(float NewWidth,float NewHeight)
{
    glBindTexture(GL_TEXTURE_2D,this->TextureIdentifer);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,NewWidth,NewHeight,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,this->TextureIdentifer,0);
    glBindRenderbuffer(GL_RENDERBUFFER,this->RenderIdentifer);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,NewWidth,NewHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,this->RenderIdentifer);
}

void SessionManager::CreateEngine(void)
{
    if (this->FirstTimeInitilized)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& InterfaceInputOutput = ImGui::GetIO();
        ImGui_ImplGlfw_InitForOpenGL(this->EngineWindow, true);
        ImGui_ImplOpenGL3_Init();

        this->FirstTimeInitilized = false;
    }

    Console::ApplyLocalTheme();
    Console::GenerateValues();

    if (std::get<std::string>(Commands::GetFromRegister("world")) != FUSION_WORLD_EMPTY)
    {
        this->SkyboxShader.CreateNewShader(
            this->EngineParser.GetDatapackData("Shader/Skybox.vert").c_str(),
            this->EngineParser.GetDatapackData("Shader/Skybox.frag").c_str()
        );

        this->SceneShader.CreateNewShader(
            this->EngineParser.GetDatapackData("Shader/Scene.vert").c_str(),
            this->EngineParser.GetDatapackData("Shader/Scene.frag").c_str()
        );

        this->EntityShader.CreateNewShader(
            this->EngineParser.GetDatapackData("Shader/Default.vert").c_str(),
            this->EngineParser.GetDatapackData("Shader/Default.frag").c_str()
        );

        this->DebugShader.CreateNewShader(
            this->EngineParser.GetDatapackData("Shader/Debug.vert").c_str(),
            this->EngineParser.GetDatapackData("Shader/Debug.frag").c_str()
        );

        this->EnginePlayer.SpawnPlayer(
            &this->EngineParser,
            "Models/Ranger.gltf", glm::vec3(0.0f, 0.0f, 0.0f),
            glm::quat(0.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.4f, 1.0f),
            ""
        );

        this->EngineSkybox.SpawnSkybox(this->Width, this->Height, this->Near, this->Far, &this->EngineParser, this->FieldOfView);
        this->EngineCamera.SpawnCamera(this->Width,this->Height,this->EnginePlayer.GetModelPosition());
        this->EnginePlayer.SetPlayerSpeed(std::get<float>(Commands::GetFromRegister("playerspeed")));
    }
}

void SessionManager::RenderEngine(void)
{
    this->UpdateEngineValue();
    this->CalculateDeltaTime();

    this->EngineCamera.MoveCamera();
    this->EngineCamera.UpdateCameraView(this->EngineWindow, this->MousePositionX, this->MousePositionY);
    this->EngineCamera.UpdateCameraMatrix(this->FieldOfView, this->Near, this->Far);
    this->EngineCamera.MoveFunctionCamera(this->EngineWindow);

    glDisable(GL_CULL_FACE);
    this->EngineSkybox.RenderSkybox(&this->EngineCamera, &this->SkyboxShader, this->FieldOfView);
    glEnable(GL_CULL_FACE);

    glfwGetCursorPos(this->EngineWindow,&this->MousePositionX,&this->MousePositionY);
}

void SessionManager::DestroyEngine(void)
{
    glfwTerminate();
    this->EngineSkybox.DestroySkybox();
    this->NewEntityService.DestroyGroup();

    this->SkyboxShader.DestroyShader();
    this->SceneShader.DestroyShader();
    this->EntityShader.DestroyShader();

    exit(0);
}