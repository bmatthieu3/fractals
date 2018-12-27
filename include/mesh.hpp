#ifndef _MESH_H_
#define _MESH_H_

#include <vector>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Include GLM core features
#include <glm/vec2.hpp>           // vec2
#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
#include <glm/trigonometric.hpp>  // radians

// Include GLM extension
#include <glm/ext/matrix_transform.hpp> // perspective, translate, rotate

#include "shader.hpp"

using namespace std;
using namespace glm;

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
};

class Mesh {
    public:
        Mesh(const shared_ptr<Shader> shader);
        ~Mesh();

        void draw(float time) const;

    private:
        // VAO
        unsigned int m_vao;
        // VBO
        unsigned int m_vbo;
        unsigned int m_ebo;

        // attributes
        mat4 m_model_mat;

        // shader
        shared_ptr<Shader> m_shader;
};

#endif