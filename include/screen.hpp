#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <vector>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.hpp"

using namespace std;

class ScreenQuad {
    public:
        ScreenQuad();
        ~ScreenQuad();

        void draw(const shared_ptr<Shader> shader, float time, float depl_x, float depl_y, float zoom) const;

    private:
        // Vertex Array Object
        unsigned int m_vao;
        // Vertex Buffer Object
        unsigned int m_vbo;
        // Element Buffer Object
        unsigned int m_ebo;
};

#endif