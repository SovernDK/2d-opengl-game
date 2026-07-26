#pragma once
#include "SDL3/SDL.h"

#include "graphics/graphics.h"
#include "graphics/texture.h"

#include "ecs/base_components.h"
#include "glm/glm.hpp"

#include "core/memory/arena.h"
#include "services/service_locator.h"
#include "debug/logging.h"
#include "graphics/rendering/renderer.h"

#include <memory>
#include <string>

using Depth = int;

class IRenderer;
class Mesh;
class MaterialInstance;

constexpr Depth UI_Z = 100;
constexpr Depth BACKGROUND_Z = 10;
constexpr Depth SPRITE_Z = 20;

class Canvas2D
{
private:
	static std::shared_ptr<IRenderer> renderer;
	static mem::Arena* arena;

	static glm::vec4 m_clip;
	static bool isClipping;
	static SDL_Color m_color;
	static Depth m_depth;
	static BlendMode m_blendMode;
public:
	static void init(std::shared_ptr<IRenderer> rendererPtr, mem::Arena* frameArena);

	static void drawTest();
	static void resize(int width, int height);

	static void drawLine(glm::vec2 pointA, glm::vec2 pointB, float lineWidth = 1);
	static void drawWireRect(glm::vec4 rect, float lineWidth = 1);
	static void drawWireRect(glm::vec2 topLeft, glm::vec2 bottomRight, float lineWidth = 1);
	static void drawWirePoly(std::vector<glm::vec2> points, float lineWidth = 1);

	static void drawSprite(ecs::Sprite& sprite, ecs::Transform2D& transform);
	static void drawImage(const TexID& texture, glm::vec4 rect, gpu::UVRect& uv);
	static void drawQuad(const glm::vec4& quad, MaterialInstance* const mat);

	static void drawText(const std::string_view text, ecs::Transform2D& transform, const std::string_view, int fontSize);
	static glm::vec2 textSize(const std::string_view text, const std::string_view fontName, int fontSize);
	static glm::vec2 textOrigin(const std::string_view text, const std::string_view fontName, int fontSize);

	static void setClipping(glm::vec4 clip) { m_clip = clip; };
	static void setIsClipping(bool clip) { isClipping = clip; };
	static void setColor(SDL_Color color) { m_color = color; };
	static void setDepth(Depth depth) { m_depth = depth; };
	static void setBlend(BlendMode blendMode) { m_blendMode = blendMode; };

	static void setInternalResolution(int width, int height);

	static void reset()
	{
		m_clip = glm::vec4(0.0f);
		isClipping = false;
		m_color = WHITE;
		m_depth = 1;
		m_blendMode = BlendMode::None;
	}

	template<typename T, typename... Args>
	static T* loadToArena(Args&&... args)
	{
		T* ptr = nullptr;
		try
		{
			ptr = arena->create<T>(std::forward<Args>(args)...);
		}
		catch (std::bad_alloc())
		{
			ErrorLog("Memory", "Out of memory trying to allocate %s!", typeid(T).name());
		};

		return ptr;
	}

private:
	static void drawMeshWithMaterial(Mesh* mesh, MaterialInstance* const instance);
	static void drawPrimitive(CommandType primitiveType, Mesh* mesh, float lineWidth = 1.0f);
};