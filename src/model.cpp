#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <map>

#include "stb_image.h"
#include "mesh.hpp"
#include "model.hpp"

static vector<shared_ptr<Texture>> loadedTextures;

Model::Model() {
}

const std::string extractDirectoryFromPath(const std::string& path) {
    std::stringstream ss(path);

    std::string token;
    std::vector<string> slicedPath;
    while(std::getline(ss, token, '/')) {
        slicedPath.push_back(token);
    }
    std::ostringstream imploded;
    std::copy(slicedPath.begin(), slicedPath.end() - 1,
        std::ostream_iterator<std::string>(imploded, "/"));

    return imploded.str();
} 

Model::Model(const shared_ptr<Shader> shader, const std::string& path): m_shader(shader) {
    // Create an instance of the Importer class
    Assimp::Importer importer;
    m_directory_path = extractDirectoryFromPath(path);

    m_scene = importer.ReadFile(path, aiProcess_Triangulate);
    // If the import failed, report it
    if(!m_scene) {
        std::cout << importer.GetErrorString() << std::endl;
        return;
    }
    // Now we can access the file's contents.
    loadNode(m_scene->mRootNode);
    // Build the animation struct
    if(m_scene->HasAnimations()) {
        setAnimation(0);
    }
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

template <typename T>
vector<size_t> sort_indexes(const vector<T> &v) {
    // initialize original index locations
    vector<size_t> idx(v.size());
    iota(idx.begin(), idx.end(), 0);

    // sort indexes based on comparing values in v
    sort(idx.begin(), idx.end(),
        [&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

    return idx;
}

void Model::createNewMesh(const aiMesh* mesh) {
    // Retrieve the vertex weight matrix indexed as : idBone x idVertex
    std::cout << mesh->mNumBones << " " << m_scene->mNumAnimations << " " << std::endl;
    
    vector<vector<float>> meshWeightMatrix = vector<vector<float>>(mesh->mNumVertices, vector<float>(mesh->mNumBones, 0.f));
    for(uint32_t i = 0; i < mesh->mNumBones; ++i) {
        aiBone* bone = mesh->mBones[i];
        string boneName(bone->mName.C_Str());
        // Insert the bone in the bones dictionary
        if(m_bones.find(boneName) == m_bones.end()) {
            m_bones.insert(pair<string, aiBone*>(boneName, bone));
        }
        
        for(uint32_t j = 0; j < bone->mNumWeights; ++j) {
            const aiVertexWeight& vertexWeight = bone->mWeights[j];

            size_t vertexId = vertexWeight.mVertexId;
            float weight = vertexWeight.mWeight;

            meshWeightMatrix[vertexId][i] = weight;
        }
    }
    
    vector<Vertex> vertices;
    
    for(uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        const aiVector3D& posVector = mesh->mVertices[i];
        glm::vec3 pos = glm::vec3(posVector.x, posVector.y, posVector.z);

        glm::vec3 normal = glm::vec3(0.f);
        if(mesh->HasNormals()) {
            const aiVector3D& normalVector = mesh->mNormals[i];
            normal = glm::vec3(normalVector.x, normalVector.y, normalVector.z);
        }

        glm::vec2 texcoord = glm::vec2(0.f);
        if(mesh->HasTextureCoords(0)) {
            const aiVector3D& texcoordVector = mesh->mTextureCoords[0][i];
            texcoord = glm::vec2(texcoordVector.x, texcoordVector.y);
        }

        // For each vertex, get the 4 bones contributing the most to that vertex
        // We get a vector<VertexWeight [4]> of size the number of vertices in the mesh
        // VertexWeight is a struct having two fields:
        // - an id of bone
        // - the weight associated with this id
        glm::ivec4 idBones = glm::ivec4(0);
        glm::vec4 weightStrongestBones = glm::vec4(0.f);
        if(mesh->HasBones()) {
            const vector<float>& weights = meshWeightMatrix[i];
            const vector<size_t>& ids = sort_indexes<float>(weights);

            vector<size_t> bonesId = vector<size_t>(4, 0);
            size_t num = std::min<size_t>(4, ids.size());
            std::copy(ids.begin(), ids.begin() + num, bonesId.begin());

            for(uint32_t k = 0; k < num; k++) {
                size_t boneId = bonesId[k];

                idBones[k] = boneId;
                weightStrongestBones[k] = weights[boneId];
            }
        }

        vertices.push_back(Vertex {pos, normal, texcoord, idBones, weightStrongestBones});
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
        string p = path.C_Str();
        int embeddedTexture = false;
        if(p[0] == '*') {
            // embedded texture
            embeddedTexture = true;
        }
    
        const string& path_str = m_directory_path + path.C_Str();

        // Check if the texture has already been loaded or not
        bool alreadyLoaded = false;
        std::shared_ptr<Texture> texture = nullptr;
        for(uint32_t j = 0; j < loadedTextures.size(); j++) {
            const shared_ptr<Texture> loadedTex = loadedTextures[j];
            if(loadedTex->path == path_str) {
                texture = loadedTex;
                alreadyLoaded = true;
                break;
            }
        }
        if(!alreadyLoaded) {
            if(!embeddedTexture) {
                texture = make_shared<Texture>(path_str, name);
            } else {
                size_t textureEmbeddedId = std::stoi(p.substr(1));
                const aiTexture* embeddedTexture = m_scene->mTextures[textureEmbeddedId];
                texture = make_shared<Texture>(embeddedTexture, path_str, name);
            }
        }
        textures.push_back(texture);
    }
    return textures;
}

void Model::draw(const Viewer& viewer, float time) {
    for(uint32_t i = 0; i < m_meshes.size(); ++i) {
        const std::unique_ptr<Mesh>& mesh = m_meshes[i];
        mesh->draw(m_shader, viewer, time);
    }
}

void Model::applyTransformation(const glm::mat4& transform) {
    for(uint32_t i = 0; i < m_meshes.size(); ++i) {
        const std::unique_ptr<Mesh>& mesh = m_meshes[i];
        mesh->applyTransformation(transform);
    }
}

void Model::update(float time) {
    if(m_anim != nullptr) {
        // Update the animation frame id
        uint32_t prevFrameId = static_cast<uint32_t>(((time - m_anim->startingTime) / m_anim->durationInSeconds) * m_anim->numFrame);
        uint32_t nextFrameId = prevFrameId + 1;

        prevFrameId = prevFrameId % m_anim->numFrame;
        nextFrameId = nextFrameId % m_anim->numFrame;

        std::cout << "prev frame " << prevFrameId << " " << nextFrameId << " num frame " << m_anim->numFrame << std::endl;

        m_anim->prevFrameId = prevFrameId;
        m_anim->nextFrameId = nextFrameId;

        const aiMatrix4x4& rootTransform = aiMatrix4x4();
        this->updateNodeTransformMatrix(m_scene->mRootNode);           
    }
}

void Model::updateNodeTransformMatrix(const aiNode* node) {    
    if(node != NULL) {
        std::cout << "kk" << std::endl;
        //string name = string(node->mName.C_Str());
        //std::cout << name << std::endl;
        //aiMatrix4x4 transform;
        // If name refers to a bone then we calculate its transform matrix
        // otherwise we just loop through its children
        
        /*if(m_bones.find(name) != m_bones.end()) {
            const aiBone* bone = m_bones[name];
            
            aiNodeAnim* nodeAnim = m_anim->nodeAnims[name];
            size_t idBone = m_anim->idBones[name];
            
            const aiVector3D& scaling = nodeAnim->mScalingKeys[1].mValue;
            const aiQuaternion& rotation = nodeAnim->mRotationKeys[1].mValue;
            const aiVector3D& position = nodeAnim->mPositionKeys[1].mValue;
            transform = aiMatrix4x4(scaling, rotation, position);

            aiMatrix4x4& finalTransform = m_anim->transforms[idBone];
            finalTransform = bone->mOffsetMatrix * transform * parentTransform;
        } else {
            transform = node->mTransformation;
        }
        aiMatrix4x4 parent = transform;
        */        
        for(uint32_t k = 0; k < node->mNumChildren; k++) {
            std::cout << k << " " << node->mChildren[k] << std::endl;
            //updateNodeTransformMatrix(node->mChildren[k]);
        }
    }
}

void Model::setAnimation(size_t index) {
    if(!m_scene->HasAnimations()) {
        cout << "The model has no animations" << std::endl;
        return;
    }

    if(index >= m_scene->mNumAnimations) {
        cout << "The model has " << m_scene->mNumAnimations << " animations" << std::endl;
        return;
    }

    std::cout << "ENTER" << std::endl;
    const aiAnimation* anim = m_scene->mAnimations[index];
    if(m_anim == nullptr) {
        m_anim = make_unique<Animation>();
    }
    m_anim->numFrame = anim->mChannels[0]->mNumPositionKeys;
    m_anim->durationInSeconds = anim->mDuration / anim->mTicksPerSecond;
    m_anim->prevFrameId = 0;
    m_anim->nextFrameId = 1 % m_anim->numFrame;

    std::map<string, aiNodeAnim*> nodeAnims;
    std::map<string, size_t> idBones;
    for(uint32_t i = 0; i < anim->mNumChannels; i++) {
        aiNodeAnim* nodeAnim = anim->mChannels[i];

        string name = string(nodeAnim->mNodeName.C_Str());
        nodeAnims.insert(std::pair<string, aiNodeAnim*>(name, nodeAnim));
        idBones.insert(std::pair<string, size_t>(name, i));
    }
    m_anim->nodeAnims = nodeAnims;
    m_anim->idBones = idBones;
    m_anim->transforms = vector<aiMatrix4x4>(anim->mNumChannels, aiMatrix4x4());
    m_anim->startingTime = glfwGetTime();
}