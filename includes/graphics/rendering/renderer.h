#pragma once
#include "graphics/camera/camera.h"
#include "graphics/rendering/render_queue.h"

class IRenderer
{
public:
	RenderQueue* commandBuffer = nullptr;
	int m_screenWidth = 800, m_screenHeight = 600;
	int m_internalWidth = 800, m_internalHeight = 600;
public:
	IRenderer() = default;
	IRenderer(const IRenderer&) = delete;
	IRenderer operator=(const IRenderer&) = delete;
	IRenderer(const IRenderer&&) = delete;
	IRenderer operator=(const IRenderer&&) = delete;
	virtual ~IRenderer() = default;

	virtual void init(int screenWidth, int screenHeight) = 0;
	virtual void render(ICamera& cam) = 0;
	virtual void resize(int width, int height) = 0;
};
