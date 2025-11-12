#include <RenderSystem.hh>
#include <Engine.hh>
#include <rttr/registration>

NAMESPACE_BEGIN

class Engine;
extern Engine* objptrGameEngine;

RTTR_REGISTRATION
{
    using namespace rttr;

    registration::class_<RenderContext>("RenderContext")
        // 注册构造函数
        .constructor<>()

        // 注册字段
        .property("VSync", &RenderContext::getVsync, &RenderContext::setVsync)
        (
        metadata("ui_type", "combo"),
        metadata("options", std::vector<std::string>{"Adaptive", "Off", "On"}),
        metadata("values", std::vector<int>{-1, 0, 1})
        )
        .property("MSAA", &RenderContext::getMSAA, &RenderContext::setMSAA)
        (
        metadata("ui_type", "combo"),
        metadata("options", std::vector<std::string>{"Off", "2x", "4x", "8x"}),
        metadata("values", std::vector<int>{0, 2, 4, 8})
        )
        .property("GL Cull Face", &RenderContext::getGlCullFace, &RenderContext::setGlCullFace)
        ;

}


RenderSystem::RenderSystem(cfgRenderSystem config, AssetSystem* assetManager)
{
    this->parseconfig(config);
    this->assetManagerPtr = assetManager;
    //init will use asset manager inside!
    this->init();
}

void RenderSystem::parseconfig(cfgRenderSystem config){
    this->config = config;

    if(config.screenmode == "window"){
        this->windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    }else if(config.screenmode == "fullscreen"){
        this->windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN ;
    }

    if(config.vsync){
        objptrAppContext->aRenderContext->vsync = 1;
    }else {
        objptrAppContext->aRenderContext->vsync = 0;
    }

    objptrAppContext->aRenderContext->windowwidth = config.screenwidth;
    objptrAppContext->aRenderContext->windowheight = config.screenheight;

}

/*On run, a RenderSystem will implicitly create a ShaderManager on init()*/
void RenderSystem::init(){
    this->rebuildWindowContext(this->firsttime_init);

    auto* geventmangager =  objptrGameEngine->getEventManager();
    auto* rctx = &*objptrAppContext->aRenderContext;

    geventmangager->subscribe(EventType::WindowResizeEvent, [rctx](const Event& e) {
        ENGINE_DEBUG("Window Resize Callback Happened!");
        int w,h;
        SDL_GL_GetDrawableSize(rctx->mainwindow,&w, &h);
        ENGINE_INFO("Window Resize Happened: w:{} h:{}", w, h);
        glViewport(0,0,w, h);
        rctx->windowheight = h;rctx->windowwidth = w;
        rctx->aCurrentCamera->setCameraDirty();
    });
}

RenderSystem::~RenderSystem(){
    SDL_GL_DeleteContext(objptrAppContext->aRenderContext->glcontext);
    SDL_DestroyWindow(objptrAppContext->aRenderContext->mainwindow);
    SDL_Quit();
}

void RenderSystem::rebuildWindowContext(bool firsttime_init) {
    GLenum errorClass;
    string error;
    if (firsttime_init) {
        SDL_Init(SDL_INIT_EVERYTHING);
        error = {SDL_GetError()};
        if(error != ""){
            ENGINE_ERROR("SDL_Init Error: %s\n", error);
        }
    }


    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    error = {SDL_GetError()};
    if(error != ""){
        ENGINE_ERROR("SDL set GL attribute Error:{}", error);
    }

    string windowtitle = objptrAppContext->aGamename + " " + objptrAppContext->aGameVersion;
    auto window = SDL_CreateWindow(windowtitle.c_str(), SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        objptrAppContext->aRenderContext->windowwidth, objptrAppContext->aRenderContext->windowheight,
        this->windowFlags
    );
    objptrAppContext->aRenderContext->mainwindow = window;
    error = {SDL_GetError()};
    if(window == NULL){
        ENGINE_ERROR("SDL_CreateWindow Error: {}", error);
    }

    if(error != ""){
        ENGINE_ERROR("SDL set GL attribute Error: {}", error);
    }

    if (firsttime_init) {
        int sdl_image_flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP  ;
        auto img_init_result = IMG_Init(sdl_image_flags);

        if(img_init_result != sdl_image_flags){
            ENGINE_DEBUG("{}\n",img_init_result);
            cout << img_init_result << endl;
            error = {IMG_GetError()};
            ENGINE_ERROR("IMG_Init Error: {}\n", error);
        }
    }

    objptrAppContext->aRenderContext->glcontext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, objptrAppContext->aRenderContext->glcontext);
    glewExperimental = GL_TRUE;

    if (firsttime_init) {
        glewInit();
        initGLDebug();
        glGetError();
    }

    int windowwidth, windowheight;
    SDL_GetWindowSize(window, &windowwidth, &windowheight);
    glViewport(0, 0, windowwidth, windowheight);


    glEnable(GL_DEPTH_TEST);

    while((errorClass = glGetError()) != GL_NO_ERROR){
        ENGINE_ERROR("GL error occur! Error code: {}", errorClass);
    }
    error = {SDL_GetError()};
    if(error != ""){
        ENGINE_ERROR("SDL set GL attribute Error: {}", error);
    }

    if (firsttime_init) {
        ENGINE_INFO("RENDER SYSTEM INIT COMPLETE");
        ENGINE_INFO("GL VERSION: {}", string{reinterpret_cast<const char*>(glGetString(GL_VERSION))});
        ENGINE_INFO("GLSL VERSION: {}", string{reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))});

        ENGINE_INFO("Register Phong Shader to ShaderFactory");
        ShaderFactory::init(this->assetManagerPtr->getShaderDir() );
        ENGINE_INFO("Creating ShaderManager...");
        this->shaderManager = new ShaderManager{};
    }


    if (firsttime_init) {
        auto vender = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        auto renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        auto version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        ENGINE_DEBUG("GL_VENDOR-GL_RENDERER-GL_VERSION: {} {} {}", vender, renderer, version);

        this->firsttime_init = false;
    }
}


//Rendering Command System
void RenderSystem::submit(const Model* model, const Shader* shader, const glm::mat4& transform){
//    if(!model->getShown()){
//        return;
//    }
    RenderCommand cmd{model, shader, transform};
    this->cmdQueue.push_back(cmd);
}


void RenderSystem::executecommand(const RenderCommand& cmd){
    this->drawmodel(*cmd.model, *cmd.shader, cmd.transform);
}

/* After config the render setting in the imgui, this method will be called to refresh the context
 * GL/SDL context may be destroy and created for new setting
 */
void RenderSystem::RefreshContext(RenderContext* rctx) {
    rctx->contextDirdy = false;
    // Window Size is handled by event manager inside RenderSystem::Init

    //MSAA
    if (rctx->MSAA_dirty) {
        SDL_GL_DeleteContext(rctx->glcontext);
        SDL_DestroyWindow(rctx->mainwindow);
        SDL_GL_ResetAttributes();

        if (rctx->msaa != 0) {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, (rctx->msaa));
        } else {
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        }

        //this will destroy all configs above! it means if you modify something, then msaa changes,
        // then you will need to reconfig everything again!!!!!
        rebuildWindowContext(false);

        if (rctx->msaa == 0)
            glDisable(GL_MULTISAMPLE);
        else
            glEnable(GL_MULTISAMPLE);
    }


    // Vertical Sync off/on
    SDL_GL_SetSwapInterval(static_cast<int>(rctx->vsync));

    //GL cull face off/on
    if (rctx->glcullface_enable) {glEnable(GL_CULL_FACE);} else {glDisable(GL_CULL_FACE);}

}


/* this method prepare the viewport & window for later rendering. UI predraw is after this method.
 * this method runs first during one framedraw. so all window/context init/changes should be made here.
 */
void RenderSystem::prerender(RenderContext* context){
    if (context->contextDirdy)
        RefreshContext(context);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera* camera = context->aCurrentCamera;
    CameraMatrices matCameraData = {camera->getViewMatrix(), camera->getProjectionMatrix(), camera->getCameraPosition()};
    //gltransform the camera matrix
    glBindBuffer(GL_UNIFORM_BUFFER, objptrAppContext->aRenderContext->uboBindings[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(matCameraData), &matCameraData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    //poll rendercommand from rctx, send it to Rendersystem
    auto& rendertargetsref = context->drawtargets;
    for (const auto& eachcommands: rendertargetsref)
    {
        submit(eachcommands.model, eachcommands.shader, eachcommands.transform);
    }
    //remember to clean the render context
    context->drawtargets.clear();
}

void RenderSystem::renderframe(RenderContext* context){
    for(RenderCommand& cmd: this->cmdQueue){
        //executing Render Command
        this->executecommand(cmd);
    }
    //clear rendering command
    this->cmdQueue.clear();
}

void RenderSystem::postrender(RenderContext* context){
    SDL_GL_SwapWindow(objptrAppContext->aRenderContext->mainwindow);
}

//Processing rendering command


/*This method implicitly active the shader in second params,
transform the transform matrix in third parm to the active shader,
and then run gl drawcalls*/
void RenderSystem::drawmodel(const Model& model, const Shader& shader, const glm::mat4& transform) const{
    for (const auto& eachmesh: model.getMeshes()) {
        for (auto& prim: eachmesh.primitives) {
            prim.drawPrimitive(shader, transform * eachmesh.localtransform , &model);
        }
    }
}

NAMESPACE_END





