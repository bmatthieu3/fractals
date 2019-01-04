#include <iostream>
#include <memory>

#include <glm/vec3.hpp>           // vec3

#include "mesh.hpp"
#include "screen.hpp"

using namespace glm;

ScreenQuad::ScreenQuad(const shared_ptr<Texture> texture):
    m_texture(texture) {
    // define the quad
    float data[] = {1.f, 1.f, 0, 1, 1,
        -1.f, 1.f, 0, 0, 1,
        -1.f, -1.f, 0, 0, 0,
        1.f, -1.f, 0, 1, 0};
    
    m_indices = vector<uint32_t>({0, 1, 2, 0, 2, 3});

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), &data[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered m_vbo as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
}

ScreenQuad::~ScreenQuad() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void ScreenQuad::draw(const shared_ptr<Shader> shader) const {
    shader->bind();

    // Send the texture to plot on the screen quad 
    glActiveTexture(GL_TEXTURE0);
    shader->sendUniform1i("tex_screen", 0);
    glBindTexture(GL_TEXTURE_2D, m_texture->id);
    
    // bind the VAO before drawing
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
