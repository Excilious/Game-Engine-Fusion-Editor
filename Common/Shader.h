#ifndef __SHADER_HEADER_FILE__
#define __SHADER_HEADER_FILE__
#define GLM_ENABLE_EXPERIMENTAL

#define VERTEX_REPORT_ID        0x000001
#define FRAGMENT_REPORT_ID      0x000002
#define PROGRAM_REPORT_ID       0x000003

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

#include "../../Resources/glad/glad.h"
#include "../../Resources/GLFW/glfw3.h"
#include "../../Resources/glm/gtc/type_ptr.hpp"
#include "Console.h"

class Shader
{
    public:
                        Shader(void);
        void            CreateNewShader(const char* VertexShaderContents,const char* FragmentShaderContents);
        void            SetVector4(const glm::vec4& Vector,const std::string& Location);
        void            SetVector3(const glm::vec3& Vector,const std::string& Location);
        void            SetVector2(const glm::vec2& Vector,const std::string& Location);
        void            SetVector1(float Vector,const std::string& Location);
        void            SetMatrix4(const glm::mat4& Matrix,const std::string& Location);
        void            ActivateShader(void)    const;
        void            DestroyShader(void)     const;
        void            ReportErrors(unsigned int Shaders,int Type);
        unsigned int    Program;
};


#endif