#include <iostream>

#include <assimp/scene.h>           // Output data structure

#include "stb_image.h"
#include "mesh.hpp"
#include "viewer.hpp"

Texture::Texture() {
}

Texture::Texture(const string& pFile, const string& textureType):
    path(pFile),
    type(textureType) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture at: " << path << std::endl;
    }
    stbi_image_free(data);
}

Texture::~Texture() {
}

Mesh::Mesh(const vector<Vertex>& vertices,
           const vector<shared_ptr<Texture>>& textures,
           const vector<uint32_t>& indices,
           const Material& material):
    m_vertices(vertices),
    m_textures(textures),
    m_indices(indices),
    m_material(material),
    m_model_mat(glm::mat4(1.0f))
 {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t), m_indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::texcoord));
    glEnableVertexAttribArray(2);

    // note that this is allowed, the call to glVertexAttribPointer registered m_vbo as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void Mesh::draw(const shared_ptr<Shader> shader, const Viewer& viewer, float time) const {
    std::cout << "Draw new mesh" << std::endl;
    shader->bind();

    shader->sendUniform1f("time", time);

    shader->sendUniformMatrix4fv("model", m_model_mat);
    shader->sendUniformMatrix4fv("view", viewer.getViewMatrix());
    shader->sendUniformMatrix4fv("projection", viewer.getProjectionMatrix());

    uint32_t idDiffuseTex = 0;
    uint32_t idSpecularTex = 0;
    uint32_t idNormalTex = 0;
    for(unsigned int i = 0; i < m_textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);

        const shared_ptr<Texture> texture = m_textures[i];
        const string& type = texture->type;
        string id;
        if(type == "tex_diffuse") {
            id = std::to_string(idDiffuseTex++);
        } else if(type == "tex_specular") {
            id = std::to_string(idSpecularTex++);
        } else if(type == "tex_normals") {
            id = std::to_string(idNormalTex++);
        }
        // Tell the GPU that the current active texture refers to the uniform "type + id"
        const string& attribute = type;
        std::cout << attribute << " " << texture->id << std::endl;
        glUniform1i(glGetUniformLocation(m_shader->getProgram(), attribute.c_str()), i);
        glBindTexture(GL_TEXTURE_2D, texture->id);
        
        /*if(attribute == "tex_diffuse") {
            glUniform1i(glGetUniformLocation(m_shader->getProgram(), attribute.c_str()), i);
            glBindTexture(GL_TEXTURE_2D, texture->id);
            break;
        }*/
    }
    // bind the VAO before drawing
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // setting back the active texture to its original
    glActiveTexture(GL_TEXTURE0);
    std::cout << "Finish draw new mesh " << idDiffuseTex << " " << idSpecularTex << " " << idNormalTex << std::endl;
}

glm::mat4& Mesh::getModelMatrix() {
    return m_model_mat;
}
