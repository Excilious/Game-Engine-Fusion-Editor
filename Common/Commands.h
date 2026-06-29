#ifndef __COMMANDS_HEADER__
#define __COMMANDS_HEADER__

#define COMMAND_TYPE_VARIANCE   int,float,bool,unsigned int,std::string
#define INVALID_VALUE           "NULL"
#define VALUE_LOCKED            true
#define VALUE_UNLOCKED          false

#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <variant>
#include <sstream>

#include "../../Resources/glad/glad.h"
#include "../../Resources/GLFW/glfw3.h"
#include "../../Resources/glm/glm.hpp"
#include "Console.h"

typedef struct
{
    std::string                             Key;
    std::variant<COMMAND_TYPE_VARIANCE>     Value;
    bool                                    Archived;
    std::string                             About;
} Register;

class Commands
{
    public:
                                                        Commands(void);
        static void                                     CreateFromRegister(const std::string& NewKey,const std::variant<COMMAND_TYPE_VARIANCE>& NewValue);
        static void                                     CreateLocalRegister(std::string& NewKey);
        static void                                     SetFromRegister(const std::string& SetKey,const std::variant<COMMAND_TYPE_VARIANCE>& SetValue);
        static void                                     EditFromRegister(std::string& SetKey);
        static std::variant<COMMAND_TYPE_VARIANCE>      GetFromRegister(const std::string& SearchKey);
        
        static void                                     PrintAllRegisters(const std::string& NewCommand);
        static void                                     ClearConsole(const std::string& NewCommand);
        static void                                     DumpConsole(const std::string& NewCommand);
        static void                                     UpdateGLValues(const std::string& NewCommand);
        static void                                     ExitGameFunction(const std::string& NewCommand);
        static std::vector<Register>                    RegisterList;
    private:
        static Register*                                GetRegisterObject(const std::string& SearchKey);
};

#endif