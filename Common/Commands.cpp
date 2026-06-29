#include "Commands.h"

std::vector<Register> Commands::RegisterList = {
    {"cullface",        1,              VALUE_UNLOCKED ,"Culls the face"},
    {"multisample",     1,              VALUE_UNLOCKED, "Multisample geometry"},
    {"depth",           1,              VALUE_UNLOCKED, "Depth test check"},
    {"blend",           1,              VALUE_UNLOCKED, "Blend function"},
    {"vsyncactive",     false,          VALUE_UNLOCKED, "Uses monitor refresh rate"},
    {"showfps",         true,           VALUE_UNLOCKED, "Shows fps"},
    {"gravity",         9.81f,          VALUE_UNLOCKED, "Gravity value"},
    {"polygon",         true,          VALUE_UNLOCKED,  "Renders in polygon mode"},
    {"exit",            false,          VALUE_UNLOCKED, "Exit check for engine"},
    {"noclip",          true,           VALUE_UNLOCKED, "Noclip"},
    {"thirdperson",     false,          VALUE_UNLOCKED, "Third person mode"},
    {"exitconfirm",     false,          VALUE_UNLOCKED, "Exits the engine"},
    {"fov",             90.0f,          VALUE_UNLOCKED, "Field of view"},
    {"near",            0.01f,           VALUE_UNLOCKED, "Near value"},
    {"width",           1920.0f,        VALUE_UNLOCKED, "Engine window width"},
    {"height",          1080.0f,        VALUE_UNLOCKED, "Engine window height"},
    {"far",             1000.0f,        VALUE_UNLOCKED, "Far value"},
    {"usefullscreen",   false,          VALUE_UNLOCKED, "Enable full screen"},
    {"playerspeed",     5.0f,           VALUE_UNLOCKED, "Player speed"},
    {"world",           "WORLDEMPTY",   VALUE_UNLOCKED, "Datapack world directory"},
    {"bsp",             "Test.bsp",     VALUE_UNLOCKED, "BSP file"},
    {"location",        "Textures",     VALUE_UNLOCKED, "Texture location"},

    {"editorfullscreen", true,          VALUE_UNLOCKED, "Is the editor full screen?"},
    {"changeskybox",    false,          VALUE_UNLOCKED, "Enable skybox editing"}
};

std::map<std::string,unsigned int> GLMap =  {
    {"cullface",    GL_CULL_FACE},
    {"depth",       GL_DEPTH_TEST},
    {"blend",       GL_BLEND},
    {"multisample", GL_MULTISAMPLE}
};

Commands::Commands(void) {};

std::variant<COMMAND_TYPE_VARIANCE> Commands::GetFromRegister(const std::string& SearchKey)
{
    int Index = -1;
    for (int RegisterIndex = 0; RegisterIndex < RegisterList.size(); RegisterIndex++) {if (RegisterList[RegisterIndex].Key == SearchKey) {Index = RegisterIndex;}}
    return ((Index == -1) ? INVALID_VALUE : RegisterList[Index].Value);
}

Register* Commands::GetRegisterObject(const std::string& SearchKey)
{
    for (int Index = 0; Index < RegisterList.size(); Index++) {if (RegisterList[Index].Key == SearchKey) {return &RegisterList[Index];}}
    return nullptr;
}

void Commands::CreateFromRegister(const std::string& NewKey,const std::variant<COMMAND_TYPE_VARIANCE>& NewValue)
{
    Register* NewRegisterSearch = Commands::GetRegisterObject(NewKey);
    if (NewRegisterSearch != nullptr) 
    { 
        Console::SysPrint("Register '" + NewKey + "' already exists. Ignoring creation command.", 2);
        return;
    }

    Register NewRegister;
    NewRegister.Key         = NewKey;
    NewRegister.Value       = NewValue;
    NewRegister.Archived    = VALUE_UNLOCKED;
    NewRegister.About       = NewKey + " runtime-created value";
    Commands::RegisterList.push_back(NewRegister);
}

void Commands::SetFromRegister(const std::string& SetKey,const std::variant<COMMAND_TYPE_VARIANCE>& NewValue)
{
    Register* NewRegister = Commands::GetRegisterObject(SetKey);
    if (NewRegister != nullptr) {NewRegister->Value = NewValue;}
}

void Commands::EditFromRegister(std::string& SetKey)
{
    if (SetKey.find("+set") != std::string::npos)
    {
        std::istringstream      NewStringStream(SetKey);
        std::string             Command,Key;
        int                     Value;
        NewStringStream >> Command >> Key >> Value;

        if (std::holds_alternative<bool>(GetFromRegister(Key)))                     {Commands::SetFromRegister(Key,((Value == 1) ? true : false));}
        else if (std::holds_alternative<int>(GetFromRegister(Key)))                 {Commands::SetFromRegister(Key,Value);}
        else if (std::holds_alternative<float>(GetFromRegister(Key)))               {Commands::SetFromRegister(Key,(float)Value);}
        else if (std::holds_alternative<std::string>(GetFromRegister(Key)))         {Commands::SetFromRegister(Key,std::to_string(Value));}
        Console::SysPrint("Successfully set '"+Key+"' to "+std::to_string(Value),1);
    }
}

void Commands::CreateLocalRegister(std::string& SetKey)
{
    if (SetKey.find("+add") != std::string::npos)
    {
        std::istringstream      NewStringStream(SetKey);
        std::string             Command,Key;
        int                     Value;
        NewStringStream >> Command >> Key >> Value;

        if (std::holds_alternative<bool>(GetFromRegister(Key)))                     {Commands::CreateFromRegister(Key,((Value == 1) ? true : false));}
        else if (std::holds_alternative<int>(GetFromRegister(Key)))                 {Commands::CreateFromRegister(Key,Value);}
        else if (std::holds_alternative<float>(GetFromRegister(Key)))               {Commands::CreateFromRegister(Key,(float)Value);}
        else if (std::holds_alternative<std::string>(GetFromRegister(Key)))         {Commands::CreateFromRegister(Key,std::to_string(Value));}
        Console::SysPrint("Successfully created '"+Key+"' using "+std::to_string(Value),1);
    }
}

void Commands::ExitGameFunction(const std::string& NewCommand)
{ }

void Commands::UpdateGLValues(const std::string& NewCommands)
{ }

void Commands::DumpConsole(const std::string& NewCommands)
{ }

void Commands::ClearConsole(const std::string& NewCommands)
{ }

void Commands::PrintAllRegisters(const std::string& NewCommands)
{ }