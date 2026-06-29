#include "Engine.h"

Engine::Engine(void)
{
    this->SceneLoaded           = false;
    this->SceneActive           = true;
    this->LastNormalTrigger     = 0.0;
    this->SelectedVertex = -1;

    this->ObjectScale           = glm::vec3(0.0f);
    this->ObjectTranslation     = glm::vec3(0.0f);
    this->ObjectRotation        = glm::vec3(0.0f);
};

void MouseViewCallback(GLFWwindow* EngineWindow, int Button, int Action, int Mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(EngineWindow, Button, Action, Mods);

    if (Button == GLFW_MOUSE_BUTTON_RIGHT && Action == GLFW_PRESS) { Console::IsConsoleOpen = false; Console::ConsoleMouseInputDetected = false; }
    else { Console::IsConsoleOpen = true; Console::ConsoleMouseInputDetected = true; }
}

float DistanceRayPoint(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 point)
{
    glm::vec3 toPoint = point - rayOrigin;
    float t = glm::dot(toPoint, rayDir);

    if (t < 0.0f) return FLT_MAX; // behind camera

    glm::vec3 closest = rayOrigin + rayDir * t;
    return glm::length(point - closest);
}

glm::vec2 WorldToScreen(glm::vec3 pos,
    glm::mat4 view,
    glm::mat4 proj,
    int width,
    int height)
{
    glm::vec4 clip = proj * view * glm::vec4(pos, 1.0);
    clip /= clip.w;

    glm::vec2 screen;
    screen.x = (clip.x * 0.5f + 0.5f) * width;
    screen.y = (1.0f - (clip.y * 0.5f + 0.5f)) * height;

    return screen;
}

int PickVertexFromMesh(
    const std::vector<PointEditor>& vertices,
    glm::vec2 mousePos,
    glm::mat4 view,
    glm::mat4 proj,
    glm::mat4 model,
    int width,
    int height)
{
    float minDist = 25.0f;
    int best = -1;

    for (int i = 0; i < vertices.size(); i++)
    {
        glm::vec3 worldPos =
            glm::vec3(model * glm::vec4(vertices[i].Position, 1.0));

        glm::vec2 screen = WorldToScreen(
            worldPos, view, proj, width, height
        );

        float d = glm::length(screen - mousePos);

        if (d < minDist)
        {
            minDist = d;
            best = i;
        }
    }

    return best;
}

glm::vec3 Engine::GetRayFromMouse(const glm::mat4& Projection, const glm::mat4& View, double MouseX, double MouseY, int ScreenWidth, int ScreenHeight)
{
    float X = (1.0f * MouseX) / ScreenWidth - 1.0f;
    float Y = 1.0f - (2.0f * MouseY) / ScreenHeight;

    glm::vec4 NearNDC = glm::vec4(X, Y, -1.0f, 1.0f);
    glm::vec4 FarNDC = glm::vec4(X, Y, 1.0f, 1.0f);
    glm::mat4 InverseVP = glm::inverse(Projection * View);

    glm::vec4 NearWorld = InverseVP * NearNDC; NearWorld /= NearWorld.w;
    glm::vec4 FarWorld = InverseVP * FarNDC;  FarWorld /= FarWorld.w;

    glm::vec3 Origin = glm::vec3(NearWorld);
    glm::vec3 Direction = glm::normalize(glm::vec3(FarWorld - NearWorld));

    return Direction;
}

Ray Engine::GetRayandDirectionFromMouse(const glm::mat4& Projection, const glm::mat4& View, double MouseX, double MouseY, int ScreenWidth, int ScreenHeight)
{
    float X = (1.0f * MouseX) / ScreenWidth - 1.0f;
    float Y = 1.0f - (2.0f * MouseY) / ScreenHeight;

    glm::vec4 NearNDC = glm::vec4(X, Y, -1.0f, 1.0f);
    glm::vec4 FarNDC = glm::vec4(X, Y, 1.0f, 1.0f);
    glm::mat4 InverseVP = glm::inverse(Projection * View);

    glm::vec4 NearWorld = InverseVP * NearNDC; NearWorld /= NearWorld.w;
    glm::vec4 FarWorld = InverseVP * FarNDC;  FarWorld /= FarWorld.w;

    glm::vec3 Origin = glm::vec3(NearWorld);
    glm::vec3 Direction = glm::normalize(glm::vec3(FarWorld - NearWorld));

    return { Origin,Direction };
}

glm::vec3 Engine::GetTriangleNormal(glm::vec3 Vertex0, glm::vec3 Vertex1, glm::vec3 Vertex2)
{
    return glm::normalize(glm::cross(Vertex1 - Vertex0, Vertex2 - Vertex0));
}

glm::vec3 Engine::GetVertex(std::vector<PointEditor> Vertices, int Index)
{
    return glm::vec3(0.0f, 0.0f, 0.0f);
}

std::vector<PointEditor> Engine::ConvertToPosition(const std::vector<float> Vertices)
{
    std::vector<PointEditor> EditorPoints = {};

    for (int Index = 0; Index < Vertices.size(); Index += 8)
    {
        PointEditor NewPoint;

        NewPoint.Position = glm::vec3(Vertices[Index], Vertices[Index + 1], Vertices[Index + 2]);
        NewPoint.Normal = glm::vec3(Vertices[Index + 3], Vertices[Index + 4], Vertices[Index + 5]);

        EditorPoints.push_back(NewPoint);
    }

    return EditorPoints;
}

std::vector<float> Engine::ConvertToVertex(const std::vector<PointEditor> Vertices, std::vector<float> OldVertices)
{
    std::vector<float> CurrentVertices = OldVertices;

    for (size_t i = 0; i < Vertices.size(); i++)
    {
        size_t baseIndex = i * 8; // 8 floats per vertex

        CurrentVertices[baseIndex + 0] = Vertices[i].Position.x;
        CurrentVertices[baseIndex + 1] = Vertices[i].Position.y;
        CurrentVertices[baseIndex + 2] = Vertices[i].Position.z;

        CurrentVertices[baseIndex + 3] = Vertices[i].Normal.x;
        CurrentVertices[baseIndex + 4] = Vertices[i].Normal.y;
        CurrentVertices[baseIndex + 5] = Vertices[i].Normal.z;

        // Optional: keep UVs or other attributes
        //CurrentVertices[baseIndex + 6] = Vertices[i].UV.x;
        //CurrentVertices[baseIndex + 7] = Vertices[i].UV.y;
    }

    return CurrentVertices;
}

std::vector<int> Engine::GetTriangleFaces(std::vector<float> Vertices, std::vector<unsigned int> Indices, int TriangleIndex)
{
    return {};
}

bool Engine::IntersectRayAABB(const glm::vec3& RayOrigin, const glm::vec3& RayDirection, const glm::vec3& BoxMin, const glm::vec3& BoxMax, float& T)
{
    float TMin = (BoxMin.x - RayOrigin.x) / RayDirection.x;
    float TMax = (BoxMax.x - RayOrigin.x) / RayDirection.x;

    if (TMin > TMax)
    {
        std::swap(TMin, TMax);
    }

    float TyMin = (BoxMin.y - RayOrigin.y) / RayDirection.y;
    float TyMax = (BoxMax.y - RayOrigin.y) / RayDirection.y;

    if (TyMin > TyMax)
    {
        std::swap(TyMin, TyMax);
    }

    if ((TMin > TyMax) || (TyMin > TMax)) return false;
    if (TyMin > TMin) TMin = TyMin;
    if (TyMax < TMax) TMax = TyMax;

    float TzMin = (BoxMin.z - RayOrigin.z) / RayDirection.z;
    float TzMax = (BoxMax.z - RayOrigin.z) / RayDirection.z;

    if (TzMin > TzMax)
    {
        std::swap(TzMin, TzMax);
    }

    if ((TMin > TzMax) || (TzMin > TMax)) return false;
    if (TzMin > TMin) TMin = TzMin;
    if (TzMax < TMax) TMax = TzMax;

    T = TMin;
    return true;
}

bool Engine::IntersectRayTriangle(const glm::vec3& Origin, const glm::vec3& Direction, const glm::vec3& TriangleV0, const glm::vec3& TriangleV1, const glm::vec3& TriangleV2,float& Triangle)
{
    const float Epsilon = 0.0000001f;

    glm::vec3 Edge1 = TriangleV1 - TriangleV0;
    glm::vec3 Edge2 = TriangleV2 - TriangleV0;
    glm::vec3 NewDirection = glm::cross(Direction, Edge2);

    float A = glm::dot(Edge1, NewDirection);

    if (fabs(A) < Epsilon)
    {
        return false;
    }

    float F = 1.0f / A;
    glm::vec3 S = Origin - TriangleV0;
    float U = F * glm::dot(S, NewDirection);

    if (U < 0.0f || U > 1.0f)
    {
        return false;
    }

    glm::vec3 Q = glm::cross(S, Edge1);
    float V = F * glm::dot(Direction, Q);

    if (V < 0.0f || U + V > 1.0f)
    {
        return false;
    }

    Triangle = F * glm::dot(Edge2, Q);
    
    return Triangle > Epsilon;
}

int Engine::PickTriangleFromMesh(const std::vector<float>& Vertices,const std::vector<unsigned int>& Indices,glm::vec3 RauOrigin,glm::vec3 RayDirection)
{ 
    return 1;
}

void Engine::SpecialKeyFunction(void)
{
    if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        if (this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex == -1) { return; }
        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_LEFT) == GLFW_PRESS)      {this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Translate += glm::vec3(-Console::IncrementValue[0], 0.0f, 0.0f);}
        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS)     {this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Translate += glm::vec3(Console::IncrementValue[0], 0.0f, 0.0f);}
        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_UP) == GLFW_PRESS)        {this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Translate += glm::vec3(0.0f, Console::IncrementValue[1], 0.0f);}
        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_DOWN) == GLFW_PRESS)      {this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Translate += glm::vec3(0.0f, -Console::IncrementValue[1], 0.0f);}

        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_PAGE_UP) == GLFW_PRESS)   {this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Translate += glm::vec3(0.0f, 0.0f, Console::IncrementValue[2]);}
        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Translate += glm::vec3(0.0f, 0.0f, -Console::IncrementValue[2]);}

        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_R) == GLFW_PRESS)
        {
            if (glfwGetTime() - this->LastNormalTrigger >= 0.4f)
            {
                glm::quat rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0));
                this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Rotate = rotation * this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Rotate;
                this->LastNormalTrigger = glfwGetTime();
            }
        }

        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_T) == GLFW_PRESS)
        {
            if (glfwGetTime() - this->LastNormalTrigger >= 0.4f)
            {
                glm::quat rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1, 0, 0));
                this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Rotate = rotation * this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Rotate;
                this->LastNormalTrigger = glfwGetTime();
            }
        }

        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_C) == GLFW_PRESS)
        {
            if (glfwGetTime() - this->LastNormalTrigger >= 0.4f)
            {
                Console::SysPrint("Copied Object", 1);
                this->NewSessionManager.NewEditorToolbar.CopyObject(this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex);
                this->LastNormalTrigger = glfwGetTime();
            }
        }

        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_V) == GLFW_PRESS)
        {
            if (glfwGetTime() - this->LastNormalTrigger >= 0.4f)
            {
                Console::SysPrint("Pased Object", 1);
                this->NewSessionManager.NewEditorToolbar.PasteObject(this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex);
                this->LastNormalTrigger = glfwGetTime();
            }
        }

        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_D) == GLFW_PRESS)
        {
            if (glfwGetTime() - this->LastNormalTrigger >= 0.4f)
            {
                Console::SysPrint("Duplicated Object", 1);
                this->NewSessionManager.NewEditorToolbar.CopyObject(this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex);
                this->NewSessionManager.NewEditorToolbar.PasteObject(this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex);
                this->LastNormalTrigger = glfwGetTime();
            }
        }

        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_B) == GLFW_PRESS)
        {
            if (glfwGetTime() - this->LastNormalTrigger >= 0.4f)
            {
                Console::EnableVertex = !Console::EnableVertex;
                if (Console::EnableVertex)
                {
                    Console::SysPrint("Vertex edit mode activated", 1);
                }
                else
                {
                    this->SelectedVertex == -1;
                    Console::SysPrint("Vertex edit mode deactivated", 1);
                }
                this->LastNormalTrigger = glfwGetTime();
            }
        }
    }

    if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_DELETE) == GLFW_PRESS)
    {
        if (glfwGetTime() - this->LastNormalTrigger >= 0.4f)
        {
            this->NewSessionManager.NewEditorToolbar.DeleteUIBlocks(this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex);
            this->LastNormalTrigger = glfwGetTime();
        }
    }

    if (!ImGui::IsAnyItemActive())
    {
        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_2) == GLFW_PRESS) { this->NewSessionManager.NewEditorToolbar.NewValues = VALUE_ROTATE; }
        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_1) == GLFW_PRESS) { this->NewSessionManager.NewEditorToolbar.NewValues = VALUE_TRANSLATE; }
        if (glfwGetKey(this->NewSessionManager.ReturnEngineWindow(), GLFW_KEY_3) == GLFW_PRESS) { this->NewSessionManager.NewEditorToolbar.NewValues = VALUE_SCALE; }
    }
}

void Engine::StartEngine(void)
{
    this->NewSessionManager.CreateSession();
    this->NewSessionManager.CreateEngine();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    this->NewSessionManager.NewEditorToolbar.NewWorkspace.UploadDebug(&this->NewDebug, &this->NewSessionManager.DebugShader);
    this->NewSessionManager.NewEditorToolbar.LoadFontImages();
    this->NewEditorViewport.CreateModelViewport(&this->NewSessionManager.EngineParser, this->NewSessionManager.GetWindowResolution().x, this->NewSessionManager.GetWindowResolution().y);

    this->NewEditorViewport.CreateFrameBuffer();
    this->NewSessionManager.CreateFrameBuffer();

    this->NewSessionManager.NewEditorToolbar.SetupImages();
    this->NewSessionManager.NewEditorToolbar.NewModelViewport.CreateModelViewport(&this->NewSessionManager.EngineParser, this->NewSessionManager.GetWindowResolution().x, this->NewSessionManager.GetWindowResolution().y);
    
    this->NewSessionManager.CreateFrameBuffer();
}

void Engine::ShowEditGizmo(int ViewportWidth, int ViewportHeight)
{
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    ImVec2 ViewportPos = ImGui::GetWindowPos();
    ImGuizmo::SetRect(ViewportPos.x, ViewportPos.y, ViewportWidth, ViewportHeight);

    if (this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex != -1 && (this->NewSessionManager.NewEditorToolbar.NewValues == VALUE_TRANSLATE || this->NewSessionManager.NewEditorToolbar.NewValues == VALUE_ROTATE || this->NewSessionManager.NewEditorToolbar.NewValues == VALUE_SCALE))
    {
        if (this->SelectedVertex != -1)
        {
            static glm::mat4 Transform;
            static glm::mat4 ModelMatrix;
           
            ImGuizmo::Style& NewStyle = ImGuizmo::GetStyle();

            if (this->SelectedVertex != this->LastVertex)
            {
                ModelMatrix = this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].TranslationMatrix *
                    this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].RotationMatrix *
                    this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].ScaleMatrix;

                glm::vec3 NewPosition = ConvertToPosition(this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Vertices)[this->SelectedVertex].Position;

                Transform       = ModelMatrix;
                Transform[3]    = ModelMatrix * glm::vec4(NewPosition, 1.0);

                this->LastVertex = this->SelectedVertex;
            }

            NewStyle.Colors[ImGuizmo::DIRECTION_X] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
            NewStyle.Colors[ImGuizmo::DIRECTION_Y] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
            NewStyle.Colors[ImGuizmo::DIRECTION_Z] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);

            ImGuizmo::Manipulate(
                glm::value_ptr(this->NewSessionManager.EngineCamera.ReturnViewMatrix()),
                glm::value_ptr(this->NewSessionManager.EngineCamera.ReturnProjectionMatrix()),
                ImGuizmo::TRANSLATE,
                ImGuizmo::WORLD,
                glm::value_ptr(Transform),
                nullptr,
                Console::IncrementValue
            );

            if (ImGuizmo::IsUsing())
            {
                static glm::mat4 InverseModel;

                if (this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex == -1) { return; }
                if (this->SelectedVertex == -1 || !Console::EnableVertex) { return; }

                if (!this->PointsInitialized)
                {
                    this->EditablePoints = ConvertToPosition(this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Vertices);
                    this->PointsInitialized = true;

                    InverseModel = glm::inverse(ModelMatrix);
                }

                glm::vec3 WorldPos = glm::vec3(Transform[3]);
                glm::vec3 LocalPos = glm::vec3(InverseModel * glm::vec4(WorldPos, 1.0f));

                this->EditablePoints[this->SelectedVertex].Position = LocalPos;
                this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Vertices = this->ConvertToVertex(this->EditablePoints, this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Vertices);

                glBindBuffer(GL_ARRAY_BUFFER, this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].VertexBuffer);
                glBufferSubData(GL_ARRAY_BUFFER, 0,
                    this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Vertices.size() * sizeof(float),
                    this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Vertices.data()
                );
            }

        }
        else if (this->SelectedVertex == -1) 
        {
            ImGuizmo::Style& NewStyle = ImGuizmo::GetStyle();

            NewStyle.Colors[ImGuizmo::DIRECTION_X] = ImVec4(0.666f, 0.000f, 0.000f, 1.000f);
            NewStyle.Colors[ImGuizmo::DIRECTION_Y] = ImVec4(0.000f, 0.666f, 0.000f, 1.000f);
            NewStyle.Colors[ImGuizmo::DIRECTION_Z] = ImVec4(0.000f, 0.000f, 0.666f, 1.000f);

            glm::mat4 ModelMatrix = this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].TranslationMatrix *
                this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].RotationMatrix *
                this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].ScaleMatrix;

            ImGuizmo::Manipulate(
                glm::value_ptr(this->NewSessionManager.EngineCamera.ReturnViewMatrix()),
                glm::value_ptr(this->NewSessionManager.EngineCamera.ReturnProjectionMatrix()),
                (this->NewSessionManager.NewEditorToolbar.NewValues == VALUE_TRANSLATE ? ImGuizmo::TRANSLATE : (this->NewSessionManager.NewEditorToolbar.NewValues == VALUE_ROTATE ? ImGuizmo::ROTATE : (this->NewSessionManager.NewEditorToolbar.NewValues == VALUE_SCALE ? ImGuizmo::SCALE : ImGuizmo::SCALE))),
                ImGuizmo::LOCAL,
                glm::value_ptr(ModelMatrix),
                nullptr,
                Console::IncrementValue
            );

            if (ImGuizmo::IsUsing())
            {
                if (glm::decompose(ModelMatrix, this->ObjectScale, this->ObjectRotation, this->ObjectTranslation, Skew, Perspective))
                {
                    if (this->ObjectScale.x <= 0.1 || this->ObjectScale.y <= 0.1 || this->ObjectScale.z <= 0.1) {return;}

                    this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Translate = this->ObjectTranslation;
                    this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Rotate = this->ObjectRotation;
                    this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Scale = this->ObjectScale;

                    this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].TranslationMatrix = glm::translate(glm::mat4(1.0f), this->ObjectTranslation);
                    this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].RotationMatrix = glm::mat4_cast(this->ObjectRotation);
                    this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].ScaleMatrix = glm::scale(glm::mat4(1.0f), this->ObjectScale);
                }
            }
        }
    }

    this->SpecialKeyFunction();
}

void Engine::RenderEngine(void)
{
    while (!glfwWindowShouldClose(this->NewSessionManager.ReturnEngineWindow()))
    {
        this->NewSessionManager.BindFrameBuffer();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glDisable(GL_CULL_FACE);

        ImGuiDockNodeFlags DockspaceFlag =
            ImGuiDockNodeFlags_NoWindowMenuButton |
            ImGuiDockNodeFlags_NoCloseButton;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        const ImGuiID DockspaceID = ImGui::GetID("MyDockspace");
        ImGui::DockSpace(DockspaceID, ImVec2(0.0f, 0.0f), DockspaceFlag);

        this->NewSessionManager.NewEditorToolbar.RenderToolbar(
            this->NewSessionManager.ReturnEngineWindow(),
            &this->NewSessionManager.EngineParser,
            &this->NewSessionManager.EngineCamera,
            this->NewSessionManager.EngineCamera.ReturnProjectionMatrix(),
            this->NewSessionManager.EngineCamera.ReturnViewMatrix(),
            this->NewSessionManager.EngineCamera.ReturnCameraPosition(),
            this->NewSessionManager.EntityShader.Program
        );

        if (!this->SceneLoaded && this->NewSessionManager.CurrentWorldFile != std::get<std::string>(Commands::GetFromRegister("world")))
        {
            this->SceneLoaded = true;
            this->NewSessionManager.NewEditorToolbar.SystemLoaded = true;
            this->NewSessionManager.UpdateWorld(std::get<std::string>(Commands::GetFromRegister("world")));
            this->NewSessionManager.CreateEngine();
        }
        else if (std::get<std::string>(Commands::GetFromRegister("world")) != FUSION_WORLD_EMPTY)
        {
            if (this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup.size() > 0 && this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex != -1)
            {
                this->NewDebug.RenderOneBoundingBoxValues(
                    this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex,
                    this->NewSessionManager.EngineCamera.ReturnProjectionMatrix(),
                    this->NewSessionManager.EngineCamera.ReturnViewMatrix(),
                    glm::translate(glm::mat4(1.0f), this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Translate),
                    glm::mat4_cast(this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Rotate),
                    glm::scale(glm::mat4(1.0f), this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Scale)
                );

                if (Console::EnableVertex)
                {
                    this->NewDebug.RenderDebugValues(
                        this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex,
                        this->NewSessionManager.EngineCamera.ReturnProjectionMatrix(),
                        this->NewSessionManager.EngineCamera.ReturnViewMatrix(),
                        glm::translate(glm::mat4(1.0f), this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Translate),
                        glm::mat4_cast(this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Rotate),
                        glm::scale(glm::mat4(1.0f), this->NewSessionManager.NewEditorToolbar.NewModelGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Scale)
                    );
                }
            }
            this->NewSessionManager.RenderEngine();
        }

        this->NewSessionManager.UnbindFrameBuffer();

        Console::RenderConsoleFrame(this->NewSessionManager.ReturnEngineWindow());
        Console::RenderDebugOverlay({
            {"Frames Per Second",(this->NewSessionManager.CurrentWorldFile == FUSION_WORLD_EMPTY ? "[Not Initilised]" : std::to_string(this->NewSessionManager.FramesPerSecond))},
            {"Frame Delta",(this->NewSessionManager.CurrentWorldFile == FUSION_WORLD_EMPTY ? "[Not Initilised]" : std::to_string(this->NewSessionManager.DeltaTime))},
            {"World Objects",std::to_string(this->NewSessionManager.NewEditorToolbar.NewModelGroup.size())},
            {"Current Camera",std::to_string(this->NewSessionManager.EngineCamera.ReturnCameraPosition().x) + "," + std::to_string(this->NewSessionManager.EngineCamera.ReturnCameraPosition().y) + "," + std::to_string(this->NewSessionManager.EngineCamera.ReturnCameraPosition().z)}
            });

        ImGui::Begin("My Scene", &this->SceneActive, ImGuiWindowFlags_NoTitleBar);
        ImGuizmo::SetOrthographic(false);

        if (this->SelectedVertex == -1)
        {
            this->PointsInitialized = false;
        }

        ImVec2 viewportPos = ImGui::GetItemRectMin();

        static double mouseX, mouseY;
        glfwGetCursorPos(this->NewSessionManager.ReturnEngineWindow(), &mouseX, &mouseY);

        glm::vec2 localMouse;
        localMouse.x = mouseX - viewportPos.x;
        localMouse.y = mouseY - viewportPos.y;

        float ViewportWidth = ImGui::GetContentRegionAvail().x;
        float ViewportHeight = ImGui::GetContentRegionAvail().y;

        this->NewSessionManager.RescaleFrameBuffer(ViewportWidth, ViewportHeight);
        this->NewSessionManager.EngineCamera.SetViewportSize(ViewportWidth, ViewportHeight, this->NewSessionManager.FieldOfView, this->NewSessionManager.Near, this->NewSessionManager.Far);
        glViewport(0, 0, ViewportWidth, ViewportHeight);
        ImVec2 Position = ImGui::GetCursorScreenPos();

        ImGui::Image((void*)this->NewSessionManager.TextureIdentifer, ImVec2(ViewportWidth, ViewportHeight), ImVec2(0, 1), ImVec2(1, 0));
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver())
        {
            if (Console::EnableVertex)
            {
                if (this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex == -1)
                {
                    Console::SysPrint("An object must be selected",2);
                }
                else
                {
                    Ray NewRay = this->GetRayandDirectionFromMouse(
                        this->NewSessionManager.EngineCamera.ReturnProjectionMatrix(),
                        this->NewSessionManager.EngineCamera.ReturnViewMatrix(),
                        this->NewSessionManager.GetCurrentMousePosition().x,
                        this->NewSessionManager.GetCurrentMousePosition().y,
                        ViewportWidth,
                        ViewportHeight
                    );


                    this->SelectedVertex = PickVertexFromMesh(
                        ConvertToPosition(this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].Vertices),
                        localMouse, this->NewSessionManager.EngineCamera.ReturnViewMatrix(), this->NewSessionManager.EngineCamera.ReturnProjectionMatrix(), this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].TranslationMatrix * this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].RotationMatrix * this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex].ScaleMatrix, ViewportWidth, ViewportHeight
                    );
                }
            }
            else
            {
                this->SelectedVertex == -1;
                glm::vec3 RayOrigin = this->NewSessionManager.EngineCamera.ReturnCameraPosition();
                glm::vec3 RayDirection = this->GetRayFromMouse(this->NewSessionManager.EngineCamera.ReturnProjectionMatrix(), this->NewSessionManager.EngineCamera.ReturnViewMatrix(), this->NewSessionManager.GetCurrentMousePosition().x, this->NewSessionManager.GetCurrentMousePosition().y, ViewportWidth, ViewportHeight);

                for (int Index = 0; Index < this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup.size(); Index++)
                {
                    glm::vec3 BoxMin = this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[Index].PreDefinedTranslation - glm::vec3(this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[Index].PreDefinedScale / glm::vec3(2.0f, 2.0f, 2.0f));
                    glm::vec3 BoxMax = this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[Index].PreDefinedTranslation + glm::vec3(this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[Index].PreDefinedScale / glm::vec3(2.0f, 2.0f, 2.0f));

                    float T;
                    if (this->IntersectRayAABB(RayOrigin, RayDirection, BoxMin, BoxMax, T))
                    {
                        glm::vec3 DragOffset = this->NewSessionManager.NewEditorToolbar.NewWorkspace.CubeEntityGroup[Index].PreDefinedTranslation - (RayOrigin + RayDirection * T);
                        this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex = Index;
                        break;
                    }
                    else
                    {
                        this->NewSessionManager.NewEditorToolbar.CurrentSelectedIndex = -1;
                        this->SelectedVertex == -1;
                    }
                }
            }
        }

        if (std::get<bool>(Commands::GetFromRegister("changeskybox")))
        {
            Console::SysPrint("Changed Image", 2);
            Commands::SetFromRegister("changeskybox", false);
            this->NewSessionManager.EngineSkybox.UpdateSkyboxBuffers(&this->NewSessionManager.EngineParser, this->NewSessionManager.NewEditorToolbar.NewWorkspace.SkyboxValues);
        }

        this->ShowEditGizmo(ViewportWidth, ViewportHeight);
        glfwSetMouseButtonCallback(this->NewSessionManager.ReturnEngineWindow(), MouseViewCallback);
        ImGui::End();

        if (this->NewSessionManager.NewEditorToolbar.GridEnable)
        {
            this->NewSessionManager.NewEditorGrid.GenerateGrid(&this->NewSessionManager.NewEditorToolbar);
            Console::GridViewEnable = true;
        }
        else
        {
            Console::GridViewEnable = false;
        }
        this->NewSessionManager.NewEditorToolbar.RenderTextureViewer(&this->NewSessionManager.EngineCamera, &this->NewSessionManager.EngineParser);
        this->NewSessionManager.NewEditorToolbar.RenderMeshProperties();
        this->NewSessionManager.NewEditorToolbar.RenderWorldView();
        this->NewEditorViewport.RenderViewport(this->NewSessionManager.ReturnEngineWindow(), &this->NewSessionManager.EngineCamera, &this->NewSessionManager.EngineParser, this->NewSessionManager.Near, this->NewSessionManager.Far, this->NewSessionManager.FieldOfView, this->NewSessionManager.NewEditorToolbar.IsModelMenuOpen);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(this->NewSessionManager.ReturnEngineWindow());
        glfwPollEvents();
    }
}

void Engine::CloseEngine(void)
{
    this->NewSessionManager.DestroyEngine();
}

int main()
{
    Engine NewEngine;
    NewEngine.StartEngine();
    NewEngine.RenderEngine();
    NewEngine.CloseEngine();
}