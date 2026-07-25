#pragma once
#include "glm/glm.hpp"
#include "SDL3/SDL.h"

using WidgetID = int;

namespace rmui
{
	struct UIRect
	{

		glm::vec2 pos = glm::vec2(0.0f);
		glm::vec2 size = glm::vec2(0.0f);

		UIRect()
		{
			pos = glm::vec2(0.0f);
			size = glm::vec2(1.0f);
		}

		template<typename T>
			requires std::is_arithmetic_v<T>
		UIRect(T v)
		{
			pos = glm::vec2(static_cast<float>(v));
			size = glm::vec2(static_cast<float>(v));
		}

		template<typename T>
			requires std::is_arithmetic_v<T>
		UIRect(T x, T y, T w, T h)
		{
			pos.x = static_cast<float>(x);
			pos.y = static_cast<float>(y);
			size.x = static_cast<float>(w);
			size.y = static_cast<float>(h);
		}

		glm::vec4 renderRect() const
		{
			return glm::vec4(pos, size);
		}
	};

	struct Margin
	{
		float left = 0;
		float right = 0;
		float top = 0;
		float bottom = 0;

		Margin(float v)
		{
			left = right = top = bottom = v;
		}
	};

	enum class DirtyFlag
	{
		NONE,
		SIZE_DIRTY,
		POS_DIRTY,
		CONTENT_DIRTY,
		ALL
	};

	enum class UIAnchor
	{
		Top_Left,
		Top_Center,
		Top_Right,
		Center,
		Bottom_Left,
		Bottom_Center,
		Bottom_Right
	};

	enum class TextAlign
	{
		Left,
		Right,
		Middle
	};

	enum class TextVertAlign
	{
		Top,
		Middle,
		Bottom
	};

	enum class TextOverflow
	{
		Shrink,
		Ellipsis
	};

	inline glm::vec2 alignText(TextAlign align, TextVertAlign valign, glm::vec2 position, glm::vec2 size, glm::vec2 tSize, float originY)
	{
		glm::vec2 out{ 0.0f };

		switch (align)
		{
		case TextAlign::Left:
			out.x = position.x;
			break;
		case TextAlign::Middle:
			out.x = position.x + (size.x - tSize.x) * 0.5f;
			break;
		case TextAlign::Right:
			out.x = position.x + (size.x - tSize.x);
			break;
		}

		switch (valign)
		{
		case TextVertAlign::Top:
			out.y = position.y + tSize.y;
			break;
		case TextVertAlign::Middle:
			out.y = position.y + (size.y + tSize.y) * 0.5f;
			break;
		case TextVertAlign::Bottom:
			out.y = position.y + size.y - originY;
			break;
		}
	}

	inline glm::vec2 uiAnchorToVec2(UIAnchor anchor)
	{
		switch (anchor)
		{
		case UIAnchor::Top_Left:   return glm::vec2(0.0f, 0.0f);
		case UIAnchor::Top_Center: return glm::vec2(0.5f, 0.0f);
		case UIAnchor::Top_Right:  return glm::vec2(1.0f, 0.0f);

		case UIAnchor::Center: return glm::vec2(0.5f, 0.5f);

		case UIAnchor::Bottom_Left:	  return glm::vec2(0.0f, 1.0f);
		case UIAnchor::Bottom_Center: return glm::vec2(0.5f, 1.0f);
		case UIAnchor::Bottom_Right:  return glm::vec2(1.0f, 1.0f);
		}
	}
}