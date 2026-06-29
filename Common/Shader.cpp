#include "Shader.h"

Shader::Shader(void) {}

void Shader::ReportErrors(unsigned int Shader,int Type)
{
    int     HasCompiled;
    char    InfoLog[1024];
    if (Type != PROGRAM_REPORT_ID)
    {
        glGetShaderiv(Shader,GL_COMPILE_STATUS,&HasCompiled);
        if (HasCompiled == 0)
        {
            glGetShaderInfoLog(Shader,1024,NULL,InfoLog);
            Console::SysPrint("Failed To Compile: "+(std::string)InfoLog,3);
            Console::SysPrint("Type Of Shader: "+(std::string)(Type == VERTEX_REPORT_ID ? "Vertex" : (Type == FRAGMENT_REPORT_ID ? "Fragment" : "Unknown")),3);
        }
    } else {
        glGetProgramiv(Shader,GL_LINK_STATUS,&HasCompiled);
        if (HasCompiled == 0)
        {
            glGetProgramInfoLog(Shader,1024,NULL,InfoLog);
            Console::SysPrint("Failed To Compile: "+(std::string)InfoLog,3);
            Console::SysPrint("Type Of Shader: Program",3);
        }
    }
}

void Shader::CreateNewShader(const char* VertexShaderContents,const char* FragmentShaderContents)
{
    unsigned int    ProgramError;

    unsigned int VertexShader   = glCreateShader(GL_VERTEX_SHADER);
    unsigned int FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(VertexShader,1,&VertexShaderContents,NULL);
    glShaderSource(FragmentShader,1,&FragmentShaderContents,NULL);

    glCompileShader(VertexShader);
    glCompileShader(FragmentShader);

    this->ReportErrors(VertexShader,VERTEX_REPORT_ID);
    this->ReportErrors(FragmentShader,FRAGMENT_REPORT_ID);

    this->Program = glCreateProgram();
    glAttachShader(this->Program,VertexShader);
    glAttachShader(this->Program,FragmentShader);
    glLinkProgram(this->Program);
    this->ReportErrors(this->Program,PROGRAM_REPORT_ID);

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
}

void Shader::SetVector4(const glm::vec4& Vector,const std::string& Location)
{glUniform4f(glGetUniformLocation(this->Program,Location.c_str()),Vector.x,Vector.y,Vector.z,Vector.w);}

void Shader::SetVector3(const glm::vec3& Vector,const std::string& Location)
{glUniform3f(glGetUniformLocation(this->Program,Location.c_str()),Vector.x,Vector.y,Vector.z);}

void Shader::SetVector2(const glm::vec2& Vector,const std::string& Location)
{glUniform2f(glGetUniformLocation(this->Program,Location.c_str()),Vector.x,Vector.y);}

void Shader::SetVector1(float Vector,const std::string& Location)
{glUniform1f(glGetUniformLocation(this->Program,Location.c_str()),Vector);}

void Shader::SetMatrix4(const glm::mat4& Matrix,const std::string& Location)
{glUniformMatrix4fv(glGetUniformLocation(this->Program,Location.c_str()),1,GL_FALSE,glm::value_ptr(Matrix));}

void Shader::ActivateShader(void) const
{glUseProgram(this->Program);}; 

void Shader::DestroyShader(void) const
{glDeleteProgram(this->Program);};

