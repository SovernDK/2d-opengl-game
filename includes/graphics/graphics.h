#pragma once
#include "glm/glm.hpp"

#define TO_GL_COORDS(rect, viewportHeight) glm::vec4(rect.x, viewportHeight - rect.y - rect.w, rect.z, rect.w)
#define AABB(a, b) gpu::checkAABB(a, b)

#define TRANSPARENT SDL_Color{ 255, 255, 255, 0 }
#define WHITE		SDL_Color{ 255, 255, 255, 255 }
#define BLACK		SDL_Color{ 0, 0, 0, 255 }
#define RED			SDL_Color{ 255, 0, 0, 255 }
#define GREEN		SDL_Color{ 0, 255, 0, 255 }
#define BLUE		SDL_Color{ 0, 0, 255, 255 }

#define Bg1		SDL_Color{ 51, 153, 255, 255 }
#define Bg2		SDL_Color{ 102, 178, 255, 255 }

#define Bg3		SDL_Color{ 204, 229, 255, 255 }
#define Bg4		SDL_Color{ 153, 204, 255, 255 }

constexpr const char* M_PROP_MAIN_COLOR = "mainColor";
constexpr const char* M_PROP_USE_TEX = "useTexture";
constexpr const char* M_PROP_TIME = "time";
constexpr const char* M_PROP_PROJECTION = "projection";
constexpr const char* M_PROP_VIEW = "view";
constexpr const char* M_PROP_MODEL = "model";

constexpr const char* M_TEX_MAIN = "image";
constexpr const char* M_TEX_NORMAL = "normal";
constexpr const char* M_TEX_RENDER = "screenTexture";

namespace gpu
{
	struct Vertex
	{
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec2 uv = glm::vec2(0.0f);
	};

	struct UVRect
	{
		float u0 = 0.0f,
			v0 = 0.0f,
			u1 = 1.0f,
			v1 = 1.0f;

		UVRect() = default;
		UVRect(float v) { u0 = v0 = u1 = v1 = v; }
		UVRect(float u0, float v0, float u1, float v1)
		{
			this->u0 = u0;
			this->v0 = v0;
			this->u1 = u1;
			this->v1 = v1;
		}
	};

	inline bool checkAABB(const glm::vec4 a, const glm::vec4 b)
	{
		return !(a.x + a.z < b.x ||   // a is left of b
			a.x > b.x + b.z ||   // a is right of b
			a.y + a.w < b.y ||   // a is above b
			a.y > b.y + b.w);    // a is below b
	}
}