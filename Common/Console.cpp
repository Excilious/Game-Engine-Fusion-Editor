#include "Console.h"
#include <iostream>

Console::Console(void) {}

bool Console::ConsoleMouseInputDetected                                             = false;

bool Console::IsConsoleOpen                                                         = true;
bool Console::ForceConsoleOpen                                                      = true;
bool Console::ConsoleToBottom                                                       = false;
bool Console::ConsoleReclaimFocus                                                   = false;
bool Console::IsDebugReferenceOpen                                                  = false;
bool Console::EnableVertex                                                          = false;
bool Console::EnableCollision                                                       = false;
bool Console::GridViewEnable                                                        = false;
float Console::MenuBarHeight                                                        = 0.0f;

float Console::IncrementValue[3]                                                    = {0.5f,0.5f,0.5f};

char Console::ConsoleInputBuffer[128];
std::unordered_map<std::string,CommandFunction> Console::ConsoleCommandFunctions    = {};
std::vector<ConsoleLogTemplate> Console::ConsoleCommandTexts                        = {};
std::vector<FunctionCommand> Console::ConsoleCommands                               = {};
ImVector<char*> Console::ConsoleRenderedItems;

void Console::AddToRenderedConsole(const char* Format, ...)
{
    char Buffer[1024];
    va_list Arguments;
    va_start(Arguments,Format);
    vsnprintf(Buffer,IM_ARRAYSIZE(Buffer),Format,Arguments);
    Buffer[IM_ARRAYSIZE(Buffer)-1] = 0;
    va_end(Arguments);
    ConsoleRenderedItems.push_back(strdup(Buffer));
}

void Console::SysPrint(const std::string& NewTextLog,int ErrorCode)
{
    glm::vec3               Colour;
    ConsoleLogTemplate      NewConsoleTemplate;
    Console::AddToRenderedConsole(((ErrorCode != 6 && ErrorCode != 7 && ErrorCode != 8 ? "[" + std::to_string(glfwGetTime()) + "]: " : "") + NewTextLog).c_str());

    if      (ErrorCode == 1) {Colour = CONSOLE_INFO;}
    else if (ErrorCode == 2) {Colour = CONSOLE_WARN;}
    else if (ErrorCode == 3) {Colour = CONSOLE_ERROR;}
    else if (ErrorCode == 4) {Colour = CONSOLE_CLIENT;}
    else if (ErrorCode == 5) {Colour = CONSOLE_SERVER;}
    else if (ErrorCode == 6) {Colour = CONSOLE_COMPILE;}
    else if (ErrorCode == 7) { Colour = CONSOLE_COMPILE_WARN;}
    else if (ErrorCode == 8) { Colour = CONSOLE_COMPILE_ERROR; }

    NewConsoleTemplate.CommandContents  = NewTextLog;
    NewConsoleTemplate.CommandColour    = Colour;
    Console::ConsoleCommandTexts.push_back(NewConsoleTemplate);
}

template <typename T>
void Console::SysPrintSafe(const T& NewLog,int ErrorCode)
{
    if constexpr(std::is_convertible_v<T,std::string>) {Console::SysPrint(std::string(NewLog),ErrorCode);}
    else 
    {
        std::ostringstream NewStringStream;
        NewStringStream << NewLog;
        Console::SysPrint(NewStringStream.str(),ErrorCode);
    }
}

void Console::AddCommand(const std::string& NewCommand,CommandFunction Function)
{
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 

    for (int Index = 0; Index < Console::ConsoleCommands.size(); Index++) 
    {if (Console::ConsoleCommands[Index].CommandString == NewCommand) {Console::SysPrint("Command '"+NewCommand+"' already exists.",2);}}

    FunctionCommand NewCommandFunction;
    NewCommandFunction.CommandString            = NewCommand;
    NewCommandFunction.ExecutableFunction       = Function;
    Console::ConsoleCommands.push_back(NewCommandFunction);
}

void Console::UpdateCommandExecution(std::string& CommandString)
{
    int Index = -1;
    for (int NewIndex = 0; NewIndex < Console::ConsoleCommands.size(); NewIndex++)  {if (Console::ConsoleCommands[NewIndex].CommandString == CommandString) {Index=NewIndex;}}
    if (Index == -1)                                                                {Console::ConsoleCommands[Index].ExecutableFunction(CommandString);}
    else if (CommandString.find("+set") != std::string::npos)                       {Commands::EditFromRegister(CommandString);}
    else if (CommandString.find("+add") != std::string::npos)                       {Commands::CreateLocalRegister(CommandString);}
    else                                                                            {Console::SysPrint("Invalid/Unknown Command",3);}
}

void Console::RenderConsoleFrame(GLFWwindow* EngineWindow)
{
    if (Console::IsConsoleOpen) {glfwSetInputMode(EngineWindow,GLFW_CURSOR,GLFW_CURSOR_NORMAL);}
    else                        {glfwSetInputMode(EngineWindow,GLFW_CURSOR,GLFW_CURSOR_DISABLED);}
 
    ImGuiDockNodeFlags DockspaceFlag =
        ImGuiDockNodeFlags_NoWindowMenuButton |
        ImGuiDockNodeFlags_PassthruCentralNode |
        ImGuiDockNodeFlags_NoCloseButton;

    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::DockSpaceOverViewport(0,ImGui::GetMainViewport(), DockspaceFlag);

    ImGui::Begin("Console",&Console::ForceConsoleOpen);
    ImGui::Separator();

    Console::ConsoleReclaimFocus    = std::get<bool>(Commands::GetFromRegister("consolefocuswhenactive"));
    const float HeightToReserve     = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

    if (ImGui::BeginChild("ScrollingRegion",ImVec2(0,-HeightToReserve),ImGuiChildFlags_None,ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1));
        for (int Index = 0; Index < Console::ConsoleRenderedItems.size(); Index++)
        {
            ImVec4  Colour;
            if (Console::ConsoleCommandTexts[Index].CommandColour == CONSOLE_INFO)              {Colour = ImVec4(1.0f,1.0f,1.0f,1.0f);}
            else if (Console::ConsoleCommandTexts[Index].CommandColour == CONSOLE_WARN)         {Colour = ImVec4(1.0f,1.0f,0.4f,1.0f);}
            else if (Console::ConsoleCommandTexts[Index].CommandColour == CONSOLE_ERROR)        {Colour = ImVec4(1.0f,0.4f,0.4f,1.0f);}
            else if (Console::ConsoleCommandTexts[Index].CommandColour == CONSOLE_SERVER)       {Colour = ImVec4(0.4f,1.0f,0.4f,1.0f);}
            else if (Console::ConsoleCommandTexts[Index].CommandColour == CONSOLE_CLIENT)       {Colour = ImVec4(0.4f,0.4f,1.0f,1.0f);}
            else if (Console::ConsoleCommandTexts[Index].CommandColour == CONSOLE_COMPILE)      {Colour = ImVec4(1.0f,1.0f,1.0f,1.0f);}
            else if (Console::ConsoleCommandTexts[Index].CommandColour == CONSOLE_COMPILE_ERROR) {Colour = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);}
            else if (Console::ConsoleCommandTexts[Index].CommandColour == CONSOLE_COMPILE_WARN) { Colour = ImVec4(1.0f, 1.0f, 0.4f, 1.0f); }
            ImGui::PushStyleColor(ImGuiCol_Text,Colour);
            ImGui::TextUnformatted(ConsoleRenderedItems[Index]);
            ImGui::PopStyleColor();
        }
        if (ConsoleToBottom || (std::get<bool>(Commands::GetFromRegister("consoleactivescroll")) && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) 
        {ImGui::SetScrollHereY(1.0f);}
        ConsoleToBottom = false;

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
    ImGui::Separator();

    ImGuiInputTextFlags TextFlags = ImGuiInputTextFlags_EnterReturnsTrue;
    if (ImGui::InputText("Input",ConsoleInputBuffer,IM_ARRAYSIZE(ConsoleInputBuffer),TextFlags))
    {
        SysPrint(ConsoleInputBuffer,1);
        if (std::string(ConsoleInputBuffer).find("+exec") == std::string::npos) 
        {
            std::string ConsoleBufferString(ConsoleInputBuffer);
            Console::UpdateCommandExecution(ConsoleBufferString);
            ConsoleInputBuffer[0]   = '\0';
            ConsoleReclaimFocus     = true;
        }
    }

    ImGui::SetItemDefaultFocus();
    if (ConsoleReclaimFocus) {ImGui::SetKeyboardFocusHere(-1);}
    ImGui::End();

    //ImGui::ShowDemoWindow();
}

void Console::RenderDebugOverlay(const std::vector<DebugData>& LocalDebugData)
{
    ImGuiWindowFlags NewDebugFlags          = ImGuiWindowFlags_AlwaysAutoResize;
    ImGuiIO& WindowInput                    = ImGui::GetIO();

    const float DebugPadding                = 10.0f;
    const ImGuiViewport* RenderViewport     = ImGui::GetMainViewport();
    ImVec2 ViewportWorkPosition             = RenderViewport->WorkPos;
    ImVec2 ViewportWorkScale                = RenderViewport->WorkSize;
    ImVec2 WindowPosition                   = ImVec2(ViewportWorkPosition.x+DebugPadding,ViewportWorkPosition.y+DebugPadding);
    ImVec2 WindowPivort                     = ImVec2(0.0f,0.0f);

    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Debug Values",&Console::IsDebugReferenceOpen,NewDebugFlags))
    {
        for (int Index = 0; Index < LocalDebugData.size(); Index++)
        {
            ImGui::Text((LocalDebugData[Index].LabelName+" | "+LocalDebugData[Index].Data).c_str());
            ImGui::Separator();
        }
    }
    ImGui::End();
}

bool Console::ShowExitMessage(void)
{
    if (std::get<bool>(Commands::GetFromRegister("exit")))
    {
        ImGui::OpenPopup("Quit?");
        ImVec2 Center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(Center,ImGuiCond_Appearing,ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Quit?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Are you sure you want to leave without saving?\nPress 'Yes' to continue.");
            ImGui::Separator();

            if (ImGui::Button("Yes", ImVec2(120, 0))) {Commands::SetFromRegister("exitconfirm",true); Commands::SetFromRegister("exit",false);}
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();  
            if (ImGui::Button("Save", ImVec2(120, 0))) { Commands::SetFromRegister("exit", false); ImGui::CloseCurrentPopup(); return true;}
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { Commands::SetFromRegister("exit", false); ImGui::CloseCurrentPopup();}
            ImGui::EndPopup();
        }
    }
    return false;
}

void Console::GenerateValues(void)
{
    Console::AddCommand("exit",Commands::ExitGameFunction);
    Console::AddCommand("-dump",Commands::DumpConsole);
    Console::AddCommand("-update",Commands::UpdateGLValues);
    Console::AddCommand("-print",Commands::PrintAllRegisters);
    Console::AddCommand("-clear",Commands::ClearConsole);

    Commands::CreateFromRegister("width",1920.0f);
    Commands::CreateFromRegister("height",1080.0f);
    Commands::CreateFromRegister("showhud",true);
    Commands::CreateFromRegister("consolefocuswhenactive",false);
    Commands::CreateFromRegister("consoleactivescroll",true);
    Commands::CreateFromRegister("gravity",-9.81f);
    Commands::CreateFromRegister("accel",15.0f);
    Commands::CreateFromRegister("decel",0.0005f);
    Commands::CreateFromRegister("speed",35.0f);
    Commands::CreateFromRegister("frict",0.006f);
    Commands::CreateFromRegister("mass",200.0f);
    Commands::CreateFromRegister("debugreference",true);
}

void Console::ApplyLocalTheme(void)
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // ===== Base Backgrounds =====
    colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.07f, 0.06f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.09f, 0.07f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.11f, 0.09f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.03f, 0.05f, 0.04f, 1.0f); // dark neon camo
    colors[ImGuiCol_DockingPreview] = ImVec4(0.00f, 1.00f, 0.45f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.04f, 0.05f, 0.05f, 1.00f);


    // ===== Borders =====
    colors[ImGuiCol_Border] = ImVec4(0.10f, 0.20f, 0.15f, 0.60f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    // ===== Frame BG =====
    colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.16f, 0.13f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.25f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.35f, 0.22f, 1.00f);

    // ===== Title Bar =====
    colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.09f, 0.07f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.20f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.07f, 0.06f, 1.00f);

    // ===== Menu Bar =====
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.07f, 0.12f, 0.09f, 1.00f);

    // ===== Buttons =====
    colors[ImGuiCol_Button] = ImVec4(0.12f, 0.22f, 0.16f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.40f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.10f, 0.65f, 0.35f, 1.00f);

    // ===== Headers (Selectable, Tree, etc) =====
    colors[ImGuiCol_Header] = ImVec4(0.10f, 0.20f, 0.15f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.15f, 0.40f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.10f, 0.70f, 0.35f, 1.00f);

    // ===== Tabs =====
    colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.14f, 0.11f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.15f, 0.35f, 0.22f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.12f, 0.50f, 0.28f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.06f, 0.10f, 0.08f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.10f, 0.25f, 0.18f, 1.00f);

    // ===== Resize Grip =====
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.10f, 0.60f, 0.30f, 0.30f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.10f, 0.70f, 0.35f, 0.80f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.10f, 0.80f, 0.40f, 1.00f);

    // ===== Scrollbar =====
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.08f, 0.07f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.12f, 0.25f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.15f, 0.45f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.10f, 0.65f, 0.35f, 1.00f);

    // ===== Checkmark & Sliders =====
    colors[ImGuiCol_CheckMark] = ImVec4(0.10f, 0.90f, 0.50f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.10f, 0.80f, 0.40f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.15f, 1.00f, 0.55f, 1.00f);

    // ===== Text =====
    colors[ImGuiCol_Text] = ImVec4(0.85f, 1.00f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.55f, 0.45f, 1.00f);

    // ===== Style Tweaks =====
    style.WindowRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.TabRounding = 4.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
}