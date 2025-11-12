#pragma once

#include <Common.hh>

#define DEFAULT_TEXTURE_NAME "defaulttexture.png"

#include <string>
#include <vector>
#include <format>
#include <optional>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <tinygltf/tiny_gltf.h>

#include <Shader.hh>
#include <Texture.hh>
#include <Logger.hh>

#include "Camera.hh"
#include "Camera.hh"

using namespace std;

NAMESPACE_BEGIN

enum class TextureType
{
    BASE_COLOR = 0,
    ROUGHNESS,
    METALIC,
    NORMAL,
    AMBIENT_OCCLUSION,
    COUNT
};
struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal = {0.0f, 0.0f, 0.1f};
    glm::vec2 TexCoords = {0.0f, 0.0f};
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Texture
{
    GLuint id;
    TextureType type;
    string type_str = {};
};

class Model;

struct Primitive
{
    int indicessizes = 0;
    int indexMaterial;
    GLuint VAO = 0, VBO = 0, EBO = 0;
    GLenum enumIndexMode = GL_TRIANGLES;

    vector<Vertex> vertices;
    vector<GLuint> indices;

    Primitive(vector<Vertex> vertices, vector<GLuint> indices, GLenum enumIndexMode, int indexMaterial) : vertices(vertices), indices(indices),
                                                                                                          indicessizes(indices.size()), enumIndexMode(enumIndexMode),
                                                                                                          indexMaterial(indexMaterial)
    {
        this->setupPrimitive();
    }

    void setupPrimitive();
    void processPrimitive();
    void drawPrimitive(const Shader &shader, const glm::mat4 transform, const Model *parentmodel) const;
};

class Model
{
public:
    class Material
    {
    public:
        vector<Texture> textures;
    };
    class Mesh
    {
    public:
        glm::mat4 localtransform;
        vector<Primitive> primitives;

        Mesh(glm::mat4 transform, vector<Primitive> primitives) : localtransform(transform), primitives(primitives)
        {
            this->setupMesh();
        };

    private:
        void setupMesh();
    };

    Model(const fs::path path, bool flipuv = true)
    {

        ENGINE_VALIDLOCATION(path);
        this->modelnameandpath = path;

        if (path.extension() != fs::path{".glb"} && path.extension() != fs::path{".gltf"})
        {
            ENGINE_ERROR("you are loading a model file with non glb/gltf extension, make sure assimp support it\n");
            return;
        }
        ENGINE_DEBUG("path after extension judge: {}", path.string());

        this->loadModel(path.string(), flipuv);
    }
    //    void Draw(const Shader& shader) const;
    // vaos, render relative infos.
    vector<Mesh> getMeshes() const { return meshes; }
    fs::path getDirectory() const { return directory; }
    glm::mat4 getTransform() const { return transform; }
    void setTransform(glm::mat4 transform) { this->transform = transform; }
    Shader *getShader() const { return shader; }
    void setShader(Shader *shader) { this->shader = shader; }
    vector<Material> getMaterials() const { return materials; }

protected:
    std::unordered_map<int, GLuint> *loadedTextures;
    glm::mat4 transform = glm::mat4(1.0f);
    vector<Mesh> meshes;
    vector<Material> materials;
    fs::path directory;
    fs::path modelnameandpath;

    Shader *shader;

    Material PBRload(tinygltf::Material &gltfmat, tinygltf::Model &gltfmodel);
    void loadModel(string path, bool flipUVy = false);
    void loadMaterials(tinygltf::Model &model);
    void processNode(tinygltf::Node *node, tinygltf::Model *model, glm::mat4 transformParent);
    optional<Model::Mesh> processMesh(const tinygltf::Node *node, const tinygltf::Model *model, glm::mat4 transform);
    Texture loadTexturefromGLB(
        tinygltf::Model &gltfmodel,
        tinygltf::Material &mat,
        int index,
        TextureType textype);

    static Texture loadDefaultTexture()
    {

        Texture texture;
        ENGINE_DEBUG("loading default texture\n");

        texture.id = TextureSdlGl{fs::path{fs::current_path() / DEFAULT_TEXTURE_NAME}.string()}.getTextureId();
        texture.type = TextureType::COUNT;
        texture.type_str = {DEFAULT_TEXTURE_NAME};

        return texture;
    }
};

NAMESPACE_END