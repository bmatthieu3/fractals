#ifndef __MODEL_HPP__
#define __MODEL_HPP__

#include <string>
#include <memory>
#include <iostream>
#include <vector>

#include <assimp/scene.h>           // Output data structure
#include <glm/mat4x4.hpp>         // mat4

#include "shader.hpp"
#include "mesh.hpp"

class Model {
    public:
        Model();
        Model(const std::shared_ptr<Shader> shader, const std::string& pFile);
        ~Model();

        void draw(const Viewer& viewer, float time);
        void applyTransformation(const glm::mat4& transform);

    private:
        void loadNode(const aiNode* node);
        void createNewMesh(const aiMesh* mesh);
        vector<shared_ptr<Texture>> loadTextureMap(const aiMaterial* material, aiTextureType type, const std::string& name) const;

    private:
        std::shared_ptr<Shader> m_shader;
        const aiScene* m_scene;
        std::string m_directory_path;

        std::vector<std::unique_ptr<Mesh>> m_meshes;
};

#endif