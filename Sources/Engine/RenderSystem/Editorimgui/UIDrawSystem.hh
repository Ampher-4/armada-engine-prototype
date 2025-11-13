#pragma once
#include <Common.hh>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL2/SDL.h>

#include <rttr/type>
#include "rttr/instance.h"

NAMESPACE_BEGIN
    class AppContext;

class UIDrawSystem {


public:
    UIDrawSystem();

    void reinitForNewContext(SDL_Window* window, SDL_GLContext glcontext);
    ~UIDrawSystem(){
        this->quit();
    }


    void prerender();

    void drawframe();

    void postrender();



private:

    void drawRenderContext();
    void DrawRTTRObject(rttr::instance obj);
    void init(SDL_Window* window, SDL_GLContext glcontext);
    void quit();


};

NAMESPACE_END