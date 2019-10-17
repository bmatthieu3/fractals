#include <string>
#include <iostream>

#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.hpp"

Shader::Shader(const std::string& vertex_filename, const std::string& fragment_filename) {
    // VERTEX shader compilation
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    this->compile(vertex_shader, vertex_filename);

    // FRAGMENT shader compilation
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    this->compile(fragment_shader, fragment_filename);

    m_program = glCreateProgram();
    glAttachShader(m_program, vertex_shader);
    glAttachShader(m_program, fragment_shader);
    glLinkProgram(m_program);

    std::cout << "Linking step" << std::endl;

    // Check for the linking step
    GLint success;
    char infoLog[512];
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(m_program, 512, NULL, infoLog);
        std::cout << "ERROR::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Once the vertex and fragment shaders have been linked
    // we can remove them
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void Shader::read_file(const std::string& filename, std::string& content) const {
    std::ifstream file(filename);
    std::string line;
    if (file.is_open()) {
        while (getline(file, line)) {
            content += line + '\n';
        }
    }

    file.close();
}

void Shader::compile(GLuint shader, const std::string& filename) const {
    std::cout << "Compilation: " << filename << std::endl;

    std::string content;
    this->read_file(filename, content);


    std::cout << content << std::endl;
    const char *c_str = content.c_str();
    glShaderSource(shader, 1, &c_str, NULL);
    glCompileShader(shader);

    // Check for the compilation success
    int  success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

Shader::~Shader() {
    glDeleteProgram(m_program);
}

void Shader::bind() const {
    glUseProgram(m_program);
}

void Shader::unbind() const {
    glUseProgram(0);
}

GLuint Shader::getProgram() const {
    return m_program;
}

void Shader::sendUniform1f(const std::string& attribute, float data) const {
    int dataLocation = glGetUniformLocation(m_program, attribute.c_str());
    if(dataLocation != -1) {
        glUniform1f(dataLocation, data);
    }
}

void Shader::sendUniform1i(const std::string& attribute, unsigned int data) const {
    int dataLocation = glGetUniformLocation(m_program, attribute.c_str());
    if(dataLocation != -1) {
        glUniform1i(dataLocation, data);
    }
}
/*
void Shader::sendUniform3f(const std::string& attribute, const glm::vec3& data) const {
    int dataLocation = glGetUniformLocation(m_program, attribute.c_str());
    if(dataLocation != -1) {
        glUniform3fv(dataLocation, 1, glm::value_ptr(data));
    } 
}

void Shader::sendUniform4f(const std::string& attribute, const glm::vec4& data) const {
    int dataLocation = glGetUniformLocation(m_program, attribute.c_str());
    if(dataLocation != -1) {
        glUniform4fv(dataLocation, 1, glm::value_ptr(data));
    }
}

void Shader::sendUniformMatrix4fv(const std::string& attribute, const glm::mat4& data) const {
    int dataLocation = glGetUniformLocation(m_program, attribute.c_str());
    if(dataLocation != -1) {
        glUniformMatrix4fv(dataLocation, 1, GL_FALSE, glm::value_ptr(data));
    }
}
*/