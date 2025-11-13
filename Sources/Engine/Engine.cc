#include <Engine.hh>


NAMESPACE_BEGIN


//global null ptr to engine instance itself
extern Engine* objptrGameEngine;
extern AppContext* objptrAppContext;

Engine::Engine(string gamename, string gameversion):
    aGamename(gamename),
    aGameversion(gameversion)
{}


/*
engine::init read config file from the working directory, 
set debug config(also output log to working directory by default) and init all sub systems*/
void Engine::init(){
    objptrGameEngine = this;

    fs::path exePath = std::filesystem::current_path() / CONFIG_FILE_NAME;
    ENGINE_VALIDLOCATION(exePath);
    this->aConfig = new Config{ exePath.string() };
    ENGINE_INFO("Config Loaded, now printing some fields:\n{}", aConfig->cfgassetsystem.assetpath);
    aAppContext = make_unique<AppContext>();
    auto* appContext = this->getAppContext();
    appContext->aRenderContext->configRender = this->aConfig->cfgrendersystem;

    objptrAppContext = getAppContext();
    aAppContext->aIsInited = true;
    aAppContext->aShouldQuit = false;
    aAppContext->aIsInited = true;
    aAppContext->aGamename = this->aGamename;
    aAppContext->aGameVersion = this->aGameversion;
    ENGINE_INFO("AppContext create success\n");

    Logger::Init(aConfig->cfgappcontext);
    aLogger = Logger::Get();
    ENGINE_INFO("Logger create success\n");
    aAssetSystem = make_unique<AssetSystem>(aConfig->cfgassetsystem);
    ENGINE_INFO("AssetSystem create success\n");
    aEventManager = std::make_unique<EventManager>();
    aInputManager = std::make_unique<InputManager>(*getEventManager());
    ENGINE_INFO("EventManager & InputManager create success\n");

    //init all systems, assign corresponding fields for later access
    aRenderSystem =  make_unique<RenderSystem>(aConfig->cfgrendersystem, getAssetSystem());
    aUIDrawSystem = make_unique<UIDrawSystem>();
    ENGINE_INFO("RenderSystem create success\n");
    ENGINE_INFO("StateManager create success\n");

    ENGINE_INFO("Starting creating ECS system cluster.....");
    aStateManager = std::make_unique<entt::registry>();
    aTransformSystem = std::make_unique<TransformSystem>(*this->aStateManager);
    aMeshSystem = std::make_unique<MeshSystem>(*this->aAppContext->aRenderContext, *this->aStateManager);
    aLightSystem = std::make_unique<LightSystem>(*this->aStateManager);
    ENGINE_INFO("ECS System mesh system created");

    ENGINE_INFO("Engine init success\n");
    ENGINE_INFO("parse some config: {}", aConfig->cfgassetsystem.assetpath);

    getEventManager()->subscribe(EventType::Quit, [this](const Event& e){
        this->aAppContext->aShouldQuit = true;
    });
}


/*
this method make sure that game logic is running under 60Hz*/
void Engine::run(EngineCallbackFunction gamelogic){
    AppContext* engineContext = getAppContext();
    ENGINE_INFO("Engine run start\n");
    ENGINE_INFO("app context content: {} {}", engineContext->aShouldQuit, engineContext->aIsInited);
    ENGINE_INFO("Engine run start\n");


    const float dt = 1.0f / 60.0f;
    Uint64 previoustime = SDL_GetTicks64();
    Uint64 accumulator = 0;
    while(!engineContext->aShouldQuit){
        //update game logic, will try best to run in 60 Hz
        this->aTransformSystem->tick(dt);
        this->aLightSystem->tick(dt);
        this->aMeshSystem->tick(dt);
        while(accumulator >= dt){

            //tick ecs
//================================================
            //update game logic
            gamelogic(dt);
            accumulator -= dt;
        }
        this->aInputManager->pollEvents();
//        ENGINE_INFO("updating camera");
        //process Input, update camera current status, like view matrix, ...
        auto* aRenderContext = this->aAppContext->aRenderContext.get();
        if(aRenderContext->aCurrentCamera != nullptr){
            aRenderContext->aCurrentCamera->UpdateCamera(dt);
        }
        //delta time field is not used currently
        aRenderContext->deltatime = dt;


      //transmit camera data, render frame, etc

//        ENGINE_DEBUG("render debug point 1: before prerend");
//        RenderSystem::errorposition(__FILE__, __LINE__);
      //transmit camera data, render frame, etc
      this->aRenderSystem->prerender(
          aRenderContext
      );
      this->aUIDrawSystem->prerender();
//        ENGINE_DEBUG("render debug point 2: before render");
//        RenderSystem::errorposition(__FILE__, __LINE__);
      this->aRenderSystem->renderframe(
          aRenderContext
      );
      if (objptrAppContext->aShowEditor)
          this->aUIDrawSystem->drawframe();
//        ENGINE_DEBUG("render debug point 3: after render");
//        RenderSystem::errorposition(__FILE__, __LINE__);
      this->aUIDrawSystem->postrender();
      this->aRenderSystem->postrender(
          aRenderContext
      );
//        ENGINE_DEBUG("render debug point 4: after postrend");
//        RenderSystem::errorposition(__FILE__, __LINE__);


        //delta accmulate
        Uint64 currenttime = SDL_GetTicks64();
        accumulator += currenttime - previoustime;
        previoustime = currenttime;
//        ENGINE_INFO("one Engine loop end\n");
    }

}

void Engine::shutdown(){
    delete this->aConfig;
    ENGINE_INFO("Engine shutdown success\n");
    Logger::Shutdown();
}



NAMESPACE_END



// (1) plain debug array new
void* operator new[](std::size_t size,
                     const char* /*name*/,
                     int         /*flags*/,
                     unsigned    /*debugFlags*/,
                     const char* /*file*/,
                     int         /*line*/)
{
    return ::operator new[](size);
}

// (2) aligned debug array new  <-- the one the linker is asking for
void* operator new[](std::size_t size,
                     std::size_t alignment,
                     std::size_t /*alignmentOffset*/,
                     const char* /*name*/,
                     int         /*flags*/,
                     unsigned    /*debugFlags*/,
                     const char* /*file*/,
                     int         /*line*/)
{
    // C++17 aligned new
    return ::operator new[](size, std::align_val_t(alignment));
}

// matching deletes (required)
void operator delete[](void* ptr,
                       const char* /*name*/,
                       int         /*flags*/,
                       unsigned    /*debugFlags*/,
                       const char* /*file*/,
                       int         /*line*/) noexcept
{
    ::operator delete[](ptr);
}

void operator delete[](void* ptr,
                       std::size_t /*size*/,
                       std::size_t /*alignment*/,
                       std::size_t /*alignmentOffset*/,
                       const char* /*name*/,
                       int         /*flags*/,
                       unsigned    /*debugFlags*/,
                       const char* /*file*/,
                       int         /*line*/) noexcept
{
    ::operator delete[](ptr);
}