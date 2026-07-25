#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

#include "graphics/rendering/renderer.h"

#include <string>
#include <memory>

namespace core
{
    struct AppState
    {
        SDL_Window* window = nullptr;

        std::string name = "Renderer";
        int width = 1920;
        int height = 1080;

        int internalWidth = 1920;
        int internalHeight = 1080;

        SDL_GLContext glContext = nullptr;
        std::shared_ptr<IRenderer> activeRenderer = nullptr;
    };
}