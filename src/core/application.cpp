#pragma once
#include "core/application.h"
#include "core/app_state.h"
#include "core/config.h"
#include "core/settings.h"
#include "core/texts.h"
#include "core/profiler.h"

#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_opengl3.h>

#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <iostream>
#include <glad/glad.h>
#include "debug/logging.h"

#include "utility/file_util.h"
#include "graphics/opengl/opengl_impl.h"
#include "graphics/rendering/canvas_2d.h"

using namespace core;

SDL_AppResult Application::Init(void** appState, int argc, char** argv)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    printCompilerInfo();

    bool configLoaded = GConfig.load(file_util::createPath("assets", "data", "config.json"));
    if (!configLoaded) {
        FatalErrorLog("application", "Unable to load config! Closing the app.");
        return SDL_APP_FAILURE;
    }

    Debug::Init(GConfig.logPath());

    bool settingsLoaded = GSettings.load(GConfig.data("settings.json"));
    if (!settingsLoaded)
    {
        FatalErrorLog("application", "Unable to load settings! Closing the app.");
		return SDL_APP_FAILURE;
    }

    std::string language = GSettings.content["language"];
	bool textLoaded = GTexts.load(GConfig.texts(std::format("{}.{}", language, "json")));
	if (!textLoaded)
	{
		FatalErrorLog("application", "Unable to load text file! Closing the app.");
		return SDL_APP_FAILURE;
	}

    AppState* context = new AppState();

	context->width = GSettings.content["video"]["windowWidth"];
	context->height = GSettings.content["video"]["windowHeight"];
	context->internalWidth = GSettings.content["video"]["internalWidth"];
	context->internalHeight = GSettings.content["video"]["internalHeight"];

    initSDL(*context);

    bool openGlInit = OpenGLImpl::init(*context, &frameArena);
    if (!openGlInit)
    {
		FatalErrorLog("application", "Unable to initialize opengl! Closing the app.");
        return SDL_APP_FAILURE;
    }

	imgui = ImGuiImpl(context->window, context->glContext, "#version 430");
    if(!imgui.init()) {
		FatalErrorLog("application", "Failed to initialize ImGui! Closing the app.");
		return SDL_APP_FAILURE;
    }

    game = Game(context->internalWidth, context->internalHeight);
    game.init(*context);
    game.arena = &frameArena;

    Profiler::instance().frameBufferCap = frameArena.capacity();

    *appState = context;

    return SDL_APP_CONTINUE;
}

SDL_AppResult Application::Iterate(void* appState)
{
    AppState* context = (AppState*)appState;

    imgui.newFrame();

    Profiler::instance().update();
    game.update(Profiler::instance().getDeltaTime());
    game.draw(Profiler::instance().getDeltaTime());

    context->activeRenderer->render(*game.mainCam.get());

	imgui.render();

    SDL_GL_SwapWindow(context->window);
    SDL_Delay(12);

    Profiler::instance().frameBufferUsed = frameArena.inUse();
    frameArena.reset();

    return SDL_APP_CONTINUE;
}

SDL_AppResult Application::Event(void* appState, SDL_Event* event)
{
    AppState* context = (AppState*)appState;

	imgui.processEvents(event);

    if (event->type == SDL_EVENT_QUIT)                                      { return SDL_APP_SUCCESS; }
    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) { return SDL_APP_SUCCESS; }

    game.input(*event, Profiler::instance().getDeltaTime());

    return SDL_APP_CONTINUE;
}

void Application::Quit(void* appState, SDL_AppResult result)
{
    AppState* context = (AppState*)appState;

	imgui.quit();
    game.quit();

    if (context)
    {
        OpenGLImpl::quit(*context);

        if (context->window)
        {
            SDL_DestroyWindow(context->window);
            context->window = nullptr;
			InfoLog("application", "Window destroyed");
            MIX_Quit();
        }

		delete context;
		InfoLog("application", "App context freed");
    }

    Debug::Destroy();
    SDL_Quit();
}

SDL_AppResult Application::initSDL(AppState& context)
{
    SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "composition");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)) {
		FatalErrorLog("application", "Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Submit click events when focusing the window.
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
    // Touch events are handled natively, no need to generate synthetic mouse events for touch devices.
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    //openGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    const float window_size_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, context.name.c_str());
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, int(context.width * window_size_scale));
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, int(context.height * window_size_scale));
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, false);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);
    context.window = SDL_CreateWindowWithProperties(props);
    SDL_DestroyProperties(props);

    if (!context.window) {
        FatalErrorLog("application", "Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

	if (!MIX_Init())
	{
		FatalErrorLog("application", "Couldn't init sound library: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

    return SDL_APP_CONTINUE;
}

void Application::printCompilerInfo()
{
    std::string cppStandard = "";

    switch (__cplusplus)
    {
    case 199711L: cppStandard = "C++98/C++03"; break;
    case 201103L: cppStandard = "C++11"; break;
    case 201402L: cppStandard = "C++14"; break;
    case 201703L: cppStandard = "C++17"; break;
    case 202002L: cppStandard = "C++20"; break;
    case 202302L: cppStandard = "C++23"; break;
    default:      cppStandard = "Unknown C++ version";
    }

    std::cout << "\033[35mC++ Standard: \033[0m" << cppStandard << "(" << __cplusplus << ")" << '\n';
#ifdef _MSC_VER
    std::cout << "\033[36mCompiler: \033[0mMSVC\n";
    std::cout << "\033[36m_MSC_VER \033[0m     = " << _MSC_VER << '\n';
    std::cout << "\033[36m_MSC_FULL_VER\033[0m = " << _MSC_FULL_VER << '\n';
#elif defined(__clang__)
    std::cout << "Compiler:\033[36m Clang \033[0m\n "
        << __clang_major__ << "."
        << __clang_minor__ << "."
        << __clang_patchlevel__ << '\n';
#elif defined(__GNUC__)
    std::cout << "Compiler:\033[36m GCC \033[0m"
        << __GNUC__ << "."
        << __GNUC_MINOR__ << "."
        << __GNUC_PATCHLEVEL__ << '\n';
#endif
}