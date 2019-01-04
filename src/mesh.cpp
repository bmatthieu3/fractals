#include <iostream>
#include <memory>

#include <assimp/scene.h>           // Output data structure
#include <glm/vec3.hpp>           // vec3

#include "stb_image.h"
#include "mesh.hpp"
#include "viewer.hpp"
#include "settings.hpp"

using namespace glm;
using namespace std;

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
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << path << " loaded" << std::endl;
    } else {
        std::cout << "Failed to load texture at: " << path << std::endl;
    }
    stbi_image_free(data);
}

Texture::Texture(const aiTexture* texture, const string& pFile, const string& textureType): 
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
    unsigned char *data = (unsigned char*)texture->pcData;

    if(texture->mHeight == 0) {
        // TODO: decode compressed images, png and jpeg
        std::cout << "uncompressed texture found " << texture->mWidth << std::endl;
    }
    if (data) {
        GLenum format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->mWidth, texture->mHeight, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << type << " loaded" << std::endl;
    } else {
        std::cout << "Failed to load texture at: " << path << std::endl;
    }
}

Texture::~Texture() {
}

const shared_ptr<Texture> Texture::createDepthMap() {
    shared_ptr<Texture> texture = make_shared<Texture>();
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    
    // Clamp the depth map to white color so that the fragments outside the 
    // the depth map of the light are never in a shadow region.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat pBorderColor[4] = {1.f, 1.f, 1.f, 1.f}; 
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, pBorderColor);

    // Nearest pixels filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    return texture;
}

Mesh::Mesh(const vector<Vertex>& vertices,
           const vector<shared_ptr<Texture>>& textures,
           const vector<uint32_t>& indices,
           const Material& material):
    m_vertices(vertices),
    m_textures(textures),
    m_indices(indices),
    m_material(material),
    m_model(glm::mat4(1.0f))
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
    glEnableVertexAttribArray(2);

    glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, idBones));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
    glEnableVertexAttribArray(4);

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

// Important: We suppose the shader is currently bound and will be unbound after the call
void Mesh::draw(const shared_ptr<Shader> shader, const Viewer& viewer) const {
    shader->sendUniformMatrix4fv("model", m_model);
    shader->sendUniformMatrix4fv("view", viewer.getViewMatrix());
    shader->sendUniformMatrix4fv("projection", viewer.getProjectionMatrix());
    shader->sendUniformMatrix4fv("nModel", glm::transpose(glm::inverse(m_model)));

    uint32_t idDiffuseTex = 1;
    uint32_t idSpecularTex = 1;
    uint32_t idNormalTex = 1;

    size_t numOffsetTextures = 1;
    for(unsigned int i = 0; i < m_textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i + numOffsetTextures);

        const shared_ptr<Texture> texture = m_textures[i];
        const string& type = texture->type;
        string id;
        if(type == "diffuse_map") {
            id = std::to_string(idDiffuseTex++);
        } else if(type == "specular_map") {
            id = std::to_string(idSpecularTex++);
        } else if(type == "normal_map") {
            id = std::to_string(idNormalTex++);
        }
        // Tell the GPU that the current active texture refers to the uniform "type + id"
        const string& attribute = type;
        shader->sendUniform1i(attribute.c_str(), i + numOffsetTextures);
        glBindTexture(GL_TEXTURE_2D, texture->id);
    }

    // Send the material to the shader
    shader->sendUniform1f("material.shininess", m_material.shininess);
    // bind the VAO before drawing
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // setting back the active texture to its original
    glActiveTexture(GL_TEXTURE0);
}

glm::mat4& Mesh::getModelMatrix() {
    return m_model;
}

std::unique_ptr<Mesh> Mesh::createPlane(const Material& material) {
    vector<Vertex> vertices({
        Vertex {vec3(0.5, 0, -0.5), vec3(0, 1, 0), vec2(0), vec4(0), vec4(0)},
        Vertex {vec3(-0.5, 0, -0.5), vec3(0, 1, 0), vec2(0), vec4(0), vec4(0)},
        Vertex {vec3(-0.5, 0, 0.5), vec3(0, 1, 0), vec2(0), vec4(0), vec4(0)},
        Vertex {vec3(0.5, 0, 0.5), vec3(0, 1, 0), vec2(0), vec4(0), vec4(0)}
    });
    vector<shared_ptr<Texture>> textures;
    vector<uint32_t> indices({
        0, 1, 2,
        0, 2, 3,
    });

    std::unique_ptr<Mesh> plane = std::make_unique<Mesh>(vertices, textures, indices, material);

    return std::move(plane);
}

void Mesh::applyTransformation(const glm::mat4& transform) {
    m_model = transform * m_model;
}