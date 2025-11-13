#pragma once
#include <string>
#include <Common.hh>

NAMESPACE_BEGIN



struct cfgRenderSystem {
    int screenwidth = 1280;
    int screenheight = 720;
    std::string screenmode = "window";
    bool vsync = false;
    int msaa = 0;
    int shadowquality = 0;
    bool glEnableFaceCulling = false;

};

struct cfgAssetSystem {
    std::string assetpath = ".";

};

struct cfgAppContext {
    //there are debug options
    bool consoleoutput = false;
    bool fileoutput = false;
    //below options false is truncate, true is append
    //above is inverse!!! true is truncate, false is append!!!!
    bool fileoutputappend = true;
};

struct cfgAudioSystem {
    bool placeholder = false;

};

struct cfgInputSystem {
    bool placeholder = false;
};

struct cfgPhysicsSystem {
    bool placeholder = false;
};
NAMESPACE_END