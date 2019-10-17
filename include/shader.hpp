#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

class Shader {
    public:
        Shader(const string& vertex_filename, const string& fragment_filename);
        ~Shader();

        void bind() const;
        void unbind() const;

        GLuint getProgram() const;

        void sendUniform1f(const std::string& attribute, float data) const;
        void sendUniform1i(const std::string& attribute, unsigned int data) const;
        //void sendUniform4f(const std::string& attribute, const glm::vec4& data) const;
        void sendUniform3f(const std::string& attribute, const glm::vec3& data) const;
        void sendUniformMatrix4fv(const std::string& attribute, const glm::mat4& data) const;

    private:
        void compile(GLuint shader, const string& filename) const;
        void read_file(const std::string& filename, std::string& content) const;

    private:
        GLuint m_program;
};

#endif