#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define CONSOLE_ERROR   glm::vec3(1.0f,0.0f,0.0f)
#define CONSOLE_WARN    glm::vec3(1.0f,1.0f,0.0f)
#define CONSOLE_INFO    glm::vec3(1.0f,1.0f,1.0f)
#define CONSOLE_CLIENT  glm::vec3(0.0f,0.0f,1.0f)
#define CONSOLE_SERVER  glm::vec3(0.0f,1.0f,0.0f)

#define CONSOLE_COMPILE glm::vec3((169.0f/255.0f),(3.0f/255.0f),(252.0f/255.0f))
#define CONSOLE_COMPILE_ERROR glm::vec3(2.0f,2.0f,2.0f)
#define CONSOLE_COMPILE_WARN glm::vec3(3.0f,3.0f,3.0f)

#define CONSOLE_EDITOR_TEXT glm::vec3(0.5f,0.25f,0.96f)
#define NOMINMAX

#include <windows.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <variant>

#include "../../Resources/imgui/imgui.h"
#include "../../Resources/imgui/imgui_impl_opengl3.h"
#include "../../Resources/imgui/imgui_impl_glfw.h"
#include "../../Resources/imgui/imgui_internal.h"
#include "../../Resources/glad/glad.h"
#include "../../Resources/glm/glm.hpp"
#include "../../Resources/GLFW/glfw3.h"
#include "Commands.h"

using CommandFunction = std::function<void(const std::string&)>;

struct ConsoleLogTemplate       {glm::vec3 CommandColour; std::string CommandContents;};
struct DebugData                {std::string LabelName; std::string Data;};

typedef struct CommandFunctions
{
    CommandFunction     ExecutableFunction;
    std::string         CommandString;
} FunctionCommand;

class Console
{
    public:
                                                                    Console(void);
        static void                                                 SysPrint(const std::string& NewTextLog,int ErrorCode);
        template<typename T> static void                            SysPrintSafe(const T& NewLog,int ErrorCode);

        static void                                                 AddCommand(const std::string& NewCommand,CommandFunction Function);
        static void                                                 UpdateCommandExecution(std::string& CommandString);
        static void                                                 RenderConsoleFrame(GLFWwindow* EngineWindow);
        static void                                                 GenerateValues(void);
        static void                                                 AddToRenderedConsole(const char* Format, ...);
        static void                                                 RenderDebugOverlay(const std::vector<DebugData>& LocalDebugData);
        static bool                                                 ShowExitMessage(void);
        static void                                                 ApplyLocalTheme(void);

        static bool                                                 IsConsoleOpen;
        static bool                                                 ConsoleMouseInputDetected;
        static bool                                                 IsDebugReferenceOpen;

        static bool                                                 EnableVertex;
        static bool                                                 EnableCollision;
        static bool                                                 GridViewEnable;
        static float                                                MenuBarHeight;

        static float                                                IncrementValue[3];
    private:
        static std::unordered_map<std::string,CommandFunction>      ConsoleCommandFunctions;
        static std::vector<ConsoleLogTemplate>                      ConsoleCommandTexts;
        static std::vector<FunctionCommand>                         ConsoleCommands;
        static char                                                 ConsoleInputBuffer[128];
        static bool                                                 ConsoleToBottom;
        static bool                                                 ConsoleReclaimFocus;
        static bool                                                 ForceConsoleOpen;
        static ImVector<char*>                                      ConsoleRenderedItems;
};

#endif