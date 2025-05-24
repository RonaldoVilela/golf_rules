#include "Shader.h"
#include "Renderer.h"

#include <glad/glad.h>
#include<sstream>
#include<fstream>

Shader::Shader(const std::string &filepath) : m_filePath(filepath), m_RendererID(0)
{
    ShaderProgramSource source = ParseShader(filepath);
    std::cout << "VERTEX" << std::endl;
    std::cout << source.VertexSource << std::endl;
    std::cout << "FRAGMENT" << std::endl;
    std::cout << source.FragmentSource << std::endl;

    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererID));
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string name, int value)
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform1iv(const std::string name, int count, const int *values)
{
    GLCall(glUniform1iv(GetUniformLocation(name), count, values));
}

void Shader::SetUniform1f(const std::string name, float value)
{
    GLCall(glUniform1f(GetUniformLocation(name), value));
}

void Shader::SetUniform4f(std::string name, float v0, float v1, float v2, float v3)
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniformMat4f(const std::string name, const glm::mat4& matrix)
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

ShaderProgramSource Shader::ParseShader(const std::string &filePath){
    std::ifstream file(filePath);

    enum class ShaderType{
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while(getline(file , line)){
        if(line.find("#shader") != std::string::npos){

            if(line.find("vertex") != std::string::npos){
                type = ShaderType::VERTEX;

            } if(line.find("fragment") != std::string::npos){
                type = ShaderType::FRAGMENT;

            }
        }
        else{
            ss[(int)type] << line << "\n";
        }
    }

    return ShaderProgramSource{ss[0].str(), ss[1].str()};
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source){
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1,&src, nullptr));
    GLCall(glCompileShader(id));
    

    int status = 0;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &status));
    if(status == GL_FALSE){
        int lenght;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &lenght));
        char message[lenght];
        GLCall(glGetShaderInfoLog(id, lenght, &lenght, message));
        std::cout << "Shader Compilation error: "<< message << "\n";

        GLCall(glDeleteShader(id));
        return 0;
    }
    return id;
}

unsigned int Shader::CreateShader(const std::string &vertexShaderSource, const std::string &fragmentShaderSource){
    GLCall(unsigned int program = glCreateProgram());
    unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLCall(glAttachShader(program, vertexShader));
    GLCall(glAttachShader(program, fragmentShader));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vertexShader));
    GLCall(glDeleteShader(fragmentShader));

    return program;

}


int Shader::GetUniformLocation(const std::string &name)
{
    if(m_UniformLocationCache.find(name) != m_UniformLocationCache.end()){
        return m_UniformLocationCache.at(name);
    }
    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if(location == -1){
        std::cout << "Warning: uniform '" << name << " doesn't exist! \n";
    }

    m_UniformLocationCache[name] = location;
    return location;
}
