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
#include "viewer.hpp"

using namespace std;
using namespace glm;

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
};

struct Material {
    float shininess;
};

struct Texture {
    Texture();
    Texture(const string& pFile, const string& textureType);
    ~Texture();

    GLuint id;
    string type;
    string path;
};

class Mesh {
    public:
        Mesh(const vector<Vertex>& vertices,
            const vector<shared_ptr<Texture>>& textures,
            const vector<uint32_t>& indices,
            const Material& material);
        ~Mesh();

        void draw(const shared_ptr<Shader> shader, const Viewer& viewer, float time) const;

        glm::mat4& getModelMatrix();

    private:
        // Vertex Array Object
        unsigned int m_vao;
        // Vertex Buffer Object
        unsigned int m_vbo;
        // Element Buffer Object
        unsigned int m_ebo;

        // Attributes
        mat4 m_model_mat;

        vector<uint32_t> m_indices;
        vector<shared_ptr<Texture>> m_textures;
        vector<Vertex> m_vertices;
        // Material
        Material m_material;
};

#endif