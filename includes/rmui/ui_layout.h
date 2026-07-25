#pragma once
#include "glm/glm.hpp"

namespace rmui
{
	class UIWidget;
	class UILabel;

	struct UIRect;
	struct Margin;
	struct StyleComponent;

	struct ILayoutStrategy
	{
		ILayoutStrategy() = default;
		ILayoutStrategy(const ILayoutStrategy&) = default;
		ILayoutStrategy& operator=(const ILayoutStrategy&) = default;

		virtual ~ILayoutStrategy() = default;

		virtual UIRect layout(UIWidget& self, const UIRect& parentRect, int index, bool last) = 0;
	};

	struct IgnoreLayout : ILayoutStrategy
	{
		UIRect layout(UIWidget& self, const UIRect& parentRect, int index, bool last) override;
	};

	enum class Expand
	{
		None, Horiz, Vert, Both
	};

	struct HorizontalLayout : ILayoutStrategy
	{
		float spacing = 0;
		Margin margin{ 0 };

		Expand expand = Expand::None;
		bool fit = false;

		UIRect layout(UIWidget& self, const UIRect& parentRect, int index, bool last) override;
	};

	struct VerticalLayout : ILayoutStrategy
	{
		float spacing = 0;
		Margin margin{ 0 };

		Expand expand = Expand::None;
		bool fit = false;

		UIRect layout(UIWidget& self, const UIRect& parentRect, int index, bool last) override;
	};

	struct AbsoluteLayout : ILayoutStrategy
	{
		UIRect layout(UIWidget& self, const UIRect& parentRect, int index, bool last) override;
	};
}
