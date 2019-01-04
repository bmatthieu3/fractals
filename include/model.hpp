#ifndef __MODEL_HPP__
#define __MODEL_HPP__

#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include <map>

#include <assimp/scene.h>           // Output data structure
#include <glm/mat4x4.hpp>         // mat4

#include "shader.hpp"
#include "mesh.hpp"

struct Animation {
    map<string, aiNodeAnim*> nodeAnims;
    uint32_t prevFrameId, nextFrameId;

    float durationInSeconds;
    float startingTime;
    uint32_t numFrame;

    map<string, size_t> idBones;
    vector<aiMatrix4x4> transforms;
};

class Model {
    public:
        Model();
        Model(const std::shared_ptr<Shader> shader, const std::string& pFile);
        ~Model();

        void update(float time);
        void updateNodeTransformMatrix(const aiNode* node);

        void draw(const Viewer& viewer);
        void applyTransformation(const glm::mat4& transform);

        void setAnimation(size_t index);

        const shared_ptr<Shader> getShader() const;

    private:
        void loadNode(const aiNode* node);
        void createNewMesh(const aiMesh* mesh);
        vector<shared_ptr<Texture>> loadTextureMap(const aiMaterial* material, aiTextureType type, const std::string& name) const;

    private:
        std::shared_ptr<Shader> m_shader;
        const aiScene* m_scene;
        std::string m_directory_path;

        std::vector<std::unique_ptr<Mesh>> m_meshes;

        // Bones indexed by their names
        std::map<string, aiBone*> m_bones;
        // Current playing animation
        unique_ptr<Animation> m_anim;
};

#endif