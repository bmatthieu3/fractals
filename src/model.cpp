#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <iostream>
#include <memory>
#include <string>

#include "stb_image.h"
#include "mesh.hpp"
#include "model.hpp"

static vector<shared_ptr<Texture>> loadedTextures;

Model::Model() {
}

Model::Model(const shared_ptr<Shader> shader, const std::string& path): m_shader(shader) {
    // Create an instance of the Importer class
    Assimp::Importer importer;
    m_scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    // If the import failed, report it
    if(!m_scene) {
        std::cout << importer.GetErrorString() << std::endl;
        return;
    }
    // Now we can access the file's contents.
    loadNode(m_scene->mRootNode);
}

Model::~Model() {
}

void Model::loadNode(const aiNode* node) {
    if(node) {
        for(uint32_t i = 0; i < node->mNumMeshes; ++i) {
            uint32_t idMesh = node->mMeshes[i];
            const aiMesh* mesh = m_scene->mMeshes[idMesh];
            createNewMesh(mesh);
        }

        for(uint32_t i = 0; i < node->mNumChildren; ++i) {
            const aiNode* child = node->mChildren[i];
            loadNode(child);
        }
    }
}

void Model::createNewMesh(const aiMesh* mesh) {
    vector<Vertex> vertices;
    for(uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        const aiVector3D& posVector = mesh->mVertices[i];
        glm::vec3 pos = glm::vec3(posVector.x, posVector.y, posVector.z);

        const aiVector3D& normalVector = mesh->mNormals[i];
        glm::vec3 normal = glm::vec3(normalVector.x, normalVector.y, normalVector.z);

        glm::vec2 texcoord = glm::vec2(0.f);
        if(mesh->mTextureCoords[0]) {
            const aiVector3D& texcoordVector = mesh->mTextureCoords[0][i];
            texcoord = glm::vec2(texcoordVector.x, texcoordVector.y);
        }

        vertices.push_back(Vertex {pos, normal, texcoord});
    }

    vector<uint32_t> indices;
    for(uint32_t i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];

        for(uint32_t j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    int32_t idMaterial = mesh->mMaterialIndex;
    vector<shared_ptr<Texture>> textures;
    Material material;
    if(idMaterial >= 0) {
        const aiMaterial* mat = m_scene->mMaterials[mesh->mMaterialIndex];
        const vector<shared_ptr<Texture>>& diffuseTextures = loadTextureMap(mat, aiTextureType_DIFFUSE, "tex_diffuse");
        textures.insert(textures.end(), diffuseTextures.begin(), diffuseTextures.end());
        const vector<shared_ptr<Texture>>& specularTextures = loadTextureMap(mat, aiTextureType_SPECULAR, "tex_specular");
        textures.insert(textures.end(), specularTextures.begin(), specularTextures.end());
        const vector<shared_ptr<Texture>>& normalsTextures = loadTextureMap(mat, aiTextureType_NORMALS, "tex_normals");
        textures.insert(textures.end(), normalsTextures.begin(), normalsTextures.end());
        
        mat->Get(AI_MATKEY_SHININESS, material.shininess);
    }
    std::unique_ptr<Mesh> newMesh = std::make_unique<Mesh>(vertices, textures, indices, material);
    m_meshes.push_back(std::move(newMesh));
}

vector<shared_ptr<Texture>> Model::loadTextureMap(const aiMaterial* material, aiTextureType type, const string& name) const {
    vector<shared_ptr<Texture>> textures;
    for(uint32_t i = 0; i < material->GetTextureCount(type); ++i) {
        aiString path;
        material->GetTexture(type, i, &path);
        const string& path_str = path.C_Str();

        // Check if the texture has already been loaded or not
        bool alreadyLoaded = false;
        std::shared_ptr<Texture> texture = nullptr;
        for(uint32_t j = 0; j < loadedTextures.size(); ++j) {
            const std::shared_ptr<Texture> loadedTexture = loadedTextures[j];
            if(loadedTexture->path == path_str) {
                alreadyLoaded = true;
                texture = loadedTexture;
                break;
            }
        }
        if(!alreadyLoaded) {
            texture = std::make_shared<Texture>(path_str, name);
            loadedTextures.push_back(texture);
        }
        textures.push_back(texture);
    }
    return textures;
}

void Model::draw(const Viewer& viewer, float time) {
    std::cout << m_meshes.size() << std::endl;
    for(uint32_t i = 0; i < 1; ++i) {
        const std::unique_ptr<Mesh>& mesh = m_meshes[i];
        mesh->draw(m_shader, viewer, time);
    }
}

void Model::applyTransformation(const glm::mat4& transform) {
    for(uint32_t i = 0; i < m_meshes.size(); ++i) {
        const std::unique_ptr<Mesh>& mesh = m_meshes[i];
        glm::mat4& model = mesh->getModelMatrix();
        model = transform * model;
    }
}
