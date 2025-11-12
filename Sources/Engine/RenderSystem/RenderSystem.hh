#pragma once

#include <Common.hh>
#include <GlobalContext.hh>
#include <ConfigTypes.hh>
#include <Shader.hh>
#include <Texture.hh>
#include <Logger.hh>
#include <Camera.hh>
#include <Model.hh>
#include <ShaderManager.hh>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <rttr/type>


//coupling to ECS currently......but same as Ogre!
#include <StageManager.hh>
#include <AssetSystem.hh>

//using EASTL for render command queue
#include <EASTL/vector.h>

NAMESPACE_BEGIN

#define DEFAULT_TEXTURE_NAME "defaulttexture.png"

extern AppContext* objptrAppContext;

#define SHADERDIR "shaders"

struct RenderCommand {
    const Model* model;
    const Shader* shader;
    glm::mat4 transform;
    int lightindex;

};


/* Render Context is created on Heap when App Context was created. it is a unique pointer, holded by app context.
 * Render System was created after rendercontext, so it is suggested to put some important args in Rcontext,
 * such as window ratio, render backend, current active camera, etc.
 */
struct RenderContext {
    bool contextDirdy = true;
    cfgRenderSystem configRender = {};

    //graphics configs
    int windowheight, windowwidth;
    int vsync;//-1 is adaptive, 0 is off, 1 is on
    int getVsync() {return vsync;}
    void setVsync(int vsync) {this->vsync = vsync;contextDirdy = true;}
    int msaa = 0;
    bool MSAA_dirty = false;

    //ctx fields
    float deltatime;
    Camera* aCurrentCamera;
    vector<GLuint> uboBindings;
    vector<RenderCommand> drawtargets;

    //render backends configs: NOT visible when game shipping!
    bool glcullface_enable = true;
    bool getGlCullFace() {return glcullface_enable;}
    void setGlCullFace(bool enable) {glcullface_enable = enable;contextDirdy = true;}
    SDL_Window* mainwindow;
    SDL_GLContext glcontext;

    //misc
    void setMSAA(int msaa) {
        this->msaa = msaa;
        this->contextDirdy = true;
        this->MSAA_dirty = true;
    }
    int getMSAA() {return this->msaa;}
    RTTR_ENABLE()
    RTTR_REGISTRATION_FRIEND
};


class FLEET_API RenderSystem {


public:
    RenderSystem(cfgRenderSystem config, AssetSystem* assetManager);
    ~RenderSystem();

    /*if logger is on, this method report all OpenGL error to logger untill no error. 
    Noticed that currently this method always return 0*/
    int static errorposition(const char* file, int line){
        GLuint errorcode = glGetError();
        if(errorcode == GL_NO_ERROR){
            ENGINE_INFO("No GL error occur");
            return 0;
        }
        while(errorcode != GL_NO_ERROR){
            ENGINE_ERROR("GL error occur! Error code: {} at {}:{}", errorcode, file, line);
            errorcode = glGetError();
        }
        return 0;
    }

//    void updatestatmanager(StateManager* stateManager);
    ShaderManager* getShaderManager() const { return shaderManager; }

    /*Noticed: this method will update the aCurrentCamera's viewmatrix field*/
    void prerender(RenderContext* context);
    void renderframe(RenderContext* context);
    void postrender(RenderContext* context);

    void initcamera(Camera* camera) {
//        this->shaderManager->bindUBO(camera);
    }

private:

void static GLAPIENTRY
OpenGLDebugMessageCallback(GLenum source,
                           GLenum type,
                           GLuint id,
                           GLenum severity,
                           GLsizei length,
                           const GLchar* message,
                           const void* userParam)
{
    // 过滤掉通知类消息
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    const char* srcStr;
    switch (source) {
        case GL_DEBUG_SOURCE_API:             srcStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   srcStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: srcStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     srcStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     srcStr = "Application"; break;
        default:                              srcStr = "Other"; break;
    }

    const char* typeStr;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
        default:                                typeStr = "Other"; break;
    }

    const char* sevStr;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         sevStr = "HIGH"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       sevStr = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW:          sevStr = "LOW"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: sevStr = "NOTIFICATION"; break;
        default:                             sevStr = "UNKNOWN"; break;
    }

    // 使用你的日志系统输出
    if (severity == GL_DEBUG_SEVERITY_HIGH || type == GL_DEBUG_TYPE_ERROR)
        ENGINE_ERROR("[GL DEBUG] Source={}, Type={}, Severity={}, Message={}",
                     srcStr, typeStr, sevStr, message);
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
        ENGINE_INFO("[GL DEBUG] Source={}, Type={}, Severity={}, Message={}",
                    srcStr, typeStr, sevStr, message);
    else
        ENGINE_DEBUG("[GL DEBUG] Source={}, Type={}, Severity={}, Message={}",
                     srcStr, typeStr, sevStr, message);
}

void static initGLDebug()
{
    // 检查是否支持 KHR_debug
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // 同步输出方便调试
        glDebugMessageCallback(OpenGLDebugMessageCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
                              0, nullptr, GL_TRUE);
        ENGINE_ERROR("[GL DEBUG] Debug output enabled.\n") ;
    }
    else {
        ENGINE_ERROR("[GL DEBUG] Context does not support debug output.\n") ;
    }
}

    void init();
    void RefreshContext(RenderContext* rctx);
    void parseconfig(cfgRenderSystem config);

    /*submit a render request on model, using model's transform*/
    void submit(const Model* model, const Shader* shader);
    /*submit a render request on model, using the transform in parameter*/
    void submit(const Model* model, const Shader* shader, const glm::mat4& transform);
    void executecommand(const RenderCommand& command);
    void rebuildWindowContext(bool firsttime_init);

    void drawmodel(const Model& model, const Shader& shader, const glm::mat4& transform) const;

    cfgRenderSystem config;
    AssetSystem* assetManagerPtr;

    int windowFlags;

    bool firsttime_init = true;

    ShaderManager* shaderManager;

    eastl::vector<RenderCommand> cmdQueue;
};







NAMESPACE_END