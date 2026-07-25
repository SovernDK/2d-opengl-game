#pragma once
#include "rmui/ui.h"
#include "rmui/ui_layout.h"
#include "rmui/ui_widget.h"

#include "services/ui_service.h"

#include <resources.h>
#include <canvas_2d.h>

using namespace rmui;

UIRect IgnoreLayout::layout(UIWidget& self, const UIRect& parentRect, int index, bool last)
{
	return self.rect;
}

UIRect AbsoluteLayout::layout(UIWidget& self, const UIRect& parentRect, int index, bool last)
{
	UIRect rect{ 0 };
	rect.pos  = parentRect.pos + self.localRect.pos * parentRect.size;
	rect.size = self.localRect.size * parentRect.size;

	rect.pos -= self.pivot * rect.size;
	rect.pos += self.offset;

	return rect;
}

UIRect HorizontalLayout::layout(UIWidget& self, const UIRect& parentRect, int index, bool last)
{
	UIRect rect{ 0 };
	rect.size = self.localRect.size * parentRect.size;

	if (expand == Expand::Both)
		rect.size.y = parentRect.size.y - margin.top - margin.bottom;

	if (fit)
	{
		int childrenSize = self.parent.lock()->children().size();
		float availableWidth = parentRect.size.x / childrenSize;
		float defSpacing = spacing * (childrenSize - 1) / childrenSize;

		rect.size.x = availableWidth - defSpacing - margin.right;
	}

	if (self.parent.expired() || index == 0)
	{
		rect.pos = glm::vec2(margin.left + parentRect.pos.x, margin.top + parentRect.pos.y);
		rect.pos += self.offset;
		return rect;
	}

	UIRect prevChild{ 0 };
	int prevChildIndex = index - 1;
	prevChild = self.parent.lock()->children().at(prevChildIndex)->rect;

	float posX = prevChild.pos.x + prevChild.size.x + spacing;
	float posY = margin.top + parentRect.pos.y;

	rect.pos = glm::vec2(posX, posY);
	rect.pos += self.offset;

	return rect;
}

UIRect VerticalLayout::layout(UIWidget& self, const UIRect& parentRect, int index, bool last)
{
	UIRect rect{ 0 };
	rect.size = self.localRect.size * parentRect.size;

	if (expand == Expand::Both)
		rect.size.x = parentRect.size.x - margin.left - margin.right;
	
	if (fit)
	{
		int childrenSize = self.parent.lock()->children().size();
		float availableHeight = parentRect.size.y / childrenSize;
		float defSpacing = spacing * (childrenSize - 1) / childrenSize;

		rect.size.y = availableHeight - defSpacing - margin.bottom;
	}

	if (self.parent.expired() || index == 0)
	{
		rect.pos = glm::vec2(margin.left + parentRect.pos.x, margin.top + parentRect.pos.y);
		rect.pos += self.offset;
		return rect;
	}

	UIRect prevChild{ 0 };
	int prevChildIndex = index - 1;
	prevChild = self.parent.lock()->children().at(prevChildIndex)->rect;

	float posX = margin.left + parentRect.pos.x;
	float posY = prevChild.pos.y + prevChild.size.y + spacing;

	rect.pos = glm::vec2(posX, posY);
	rect.pos += self.offset;

	return rect;
}