#ifndef _SCREEN_H_
#define _SCREEN_H_

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
#include "mesh.hpp"

using namespace std;
using namespace glm;

class ScreenQuad {
    public:
        ScreenQuad(const shared_ptr<Texture> texture);
        ~ScreenQuad();

        void draw(const shared_ptr<Shader> shader) const;

    private:
        // Vertex Array Object
        unsigned int m_vao;
        // Vertex Buffer Object
        unsigned int m_vbo;
        // Element Buffer Object
        unsigned int m_ebo;

        vector<uint32_t> m_indices;
        shared_ptr<Texture> m_texture;
};

#endif