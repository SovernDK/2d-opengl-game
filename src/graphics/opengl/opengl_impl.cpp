#pragma once
#include "graphics/opengl/opengl_impl.h"
#include "graphics/opengl/opengl_renderer.h"
#include "graphics/rendering/canvas_2d.h"
#include "core/memory/arena.h"
#include "debug/logging.h"

#include <iostream>
#include <SDL3/SDL_log.h>

bool OpenGLImpl::init(core::AppState& context, mem::Arena* frameArena)
{
    bool openGlInitialized = openGl(context);
    if (!openGlInitialized)
    {
        return false;
    }

    bool rendererInitialized = initRenderer(context, frameArena);
    if (!rendererInitialized)
    {
        return false;
    }

    return true;
}

bool OpenGLImpl::openGl(core::AppState& context)
{
    //openGL context
    context.glContext = SDL_GL_CreateContext(context.window);
    if (!context.glContext)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "GL context creation failed");
        return false;
    }

    SDL_GL_MakeCurrent(context.window, context.glContext);
    SDL_GL_SetSwapInterval(1);

    //glad initialization
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to init GLAD");
        return false;
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    //openGL Debug Output
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        //glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        SDL_Log("\033[32m OpenGL Debug output initialized! \033[0m");
    }
    else
    {
        SDL_Log("Error trying to initialize Debug Output");
        return false;
    }

    SDL_Log("\033[32m OpenGL version \033[0m: %s", glGetString(GL_VERSION));

    return true;
}

bool OpenGLImpl::initRenderer(core::AppState& context, mem::Arena* frameArena)
{
    context.activeRenderer = std::make_shared<OpenGlRenderer>();
    context.activeRenderer->init(context.width, context.height);

    Canvas2D::init(context.activeRenderer, frameArena);

    return true;
}

void OpenGLImpl::quit(core::AppState& context)
{
    if (context.glContext)
    {
        SDL_GL_DestroyContext(context.glContext);
        context.glContext = nullptr;
        InfoLog("OpenGL", "OpenGL context destroyed");
    }

    if (context.activeRenderer)
    {
        context.activeRenderer.reset();
    }
}

// OpenGL Debug Output
void APIENTRY OpenGLImpl::glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "\033[31mDebug message \033[0m (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}