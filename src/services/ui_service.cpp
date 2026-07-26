#pragma once
#include "services/ui_service.h"
#include "graphics/material.h"
#include "utility/utils.h"

#include "services/service_locator.h"
#include "services/input_service.h"
#include "services/log_service.h"
#include "graphics/graphics.h"

#include "magic_enum/magic_enum.hpp"
#include "graphics/rendering/canvas_2d.h"

using namespace rmui;
using json = nlohmann::json;

void UIService::loadStyles(json data)
{
	for (auto& [key, val] : data.items())
	{
		std::unique_ptr<StyleComponent> style = std::make_unique<StyleComponent>();
		style->name = key;
		style->alpha = val.value("alpha", 255);

		if (val.contains("background"))
		{
			auto& backVal = val["background"];

			auto color = backVal.value("color", "#000000");
			auto hoverColor = backVal.value("hoverColor", "#000000");
			auto textureHandle = backVal.value("texture", "0");

			style->back = rmui::StyleBackground();
			style->back.keepAspectRatio = backVal.value("keepAspectRatio", false);
			if(textureHandle != "0")
				style->back.texture = Resources::texture(textureHandle)->id;
			style->back.color = HexToRGBA(color, style->alpha);
			style->back.hoverColor = HexToRGBA(hoverColor, style->alpha);
		}

		if (val.contains("text"))
		{
			auto& textVal = val["text"];

			auto color = textVal.value("color", "#000000");
			auto hoverColor = textVal.value("hoverColor", "#000000");
			auto textOverflow = textVal.value("overflow", "ellipsis");
			auto textAlign = textVal.value("align", "left");
			auto vtextAlign = textVal.value("valign", "top");

			style->text = rmui::StyleText();
			style->text.font = textVal.value("font", "default");
			style->text.size = textVal.value("size", 24);
			style->text.color = HexToRGBA(color, style->alpha);
			style->text.hoverColor = HexToRGBA(hoverColor, style->alpha);

			style->text.overflow = magic_enum::enum_cast<TextOverflow>(textOverflow, magic_enum::case_insensitive)
				.value_or(TextOverflow::Ellipsis);
			style->text.align = magic_enum::enum_cast<TextAlign>(textAlign, magic_enum::case_insensitive)
				.value_or(TextAlign::Left);
			style->text.valign = magic_enum::enum_cast<TextVertAlign>(vtextAlign, magic_enum::case_insensitive)
				.value_or(TextVertAlign::Top);
		}

		if (val.contains("horizontalLayout"))
		{
			Margin margin{ 0 };
			margin.left		= val["horizontalLayout"]["margin"].value("left", 0);
			margin.right	= val["horizontalLayout"]["margin"].value("right", 0);
			margin.top		= val["horizontalLayout"]["margin"].value("top", 0);
			margin.bottom	= val["horizontalLayout"]["margin"].value("bottom", 0);

			std::unique_ptr<HorizontalLayout> layout = std::make_unique<HorizontalLayout>();

			layout->margin	= margin;
			layout->spacing = val["horizontalLayout"].value("spacing", 0);
			//layout->expand = val["horizontalLayout"].value("expand", false);

			auto expand = val["horizontalLayout"]["margin"].value("expand", "both");
			layout->expand = magic_enum::enum_cast<Expand>(expand, magic_enum::case_insensitive)
				.value_or(Expand::None);

			layout->fit		= val["horizontalLayout"].value("fit", false);

			style->layoutStrategy = std::move(layout);
		}
		else if (val.contains("verticalLayout"))
		{
			Margin margin{ 0 };
			margin.left		= val["verticalLayout"]["margin"].value("left", 0);
			margin.right	= val["verticalLayout"]["margin"].value("right", 0);
			margin.top		= val["verticalLayout"]["margin"].value("top", 0);
			margin.bottom	= val["verticalLayout"]["margin"].value("bottom", 0);

			style->layoutStrategy = std::make_unique<VerticalLayout>();

			std::unique_ptr<VerticalLayout> layout = std::make_unique<VerticalLayout>();

			layout->margin	= margin;
			layout->spacing = val["verticalLayout"].value("spacing", 0);
			
			//layout->expand	= val["verticalLayout"].value("expand", false);
			auto expand = val["verticalLayout"]["margin"].value("expand", "both");
			layout->expand = magic_enum::enum_cast<Expand>(expand, magic_enum::case_insensitive)
				.value_or(Expand::None);

			layout->fit		= val["verticalLayout"].value("fit", false);

			style->layoutStrategy = std::move(layout);
		}
		else
		{
			style->layoutStrategy = std::make_unique<AbsoluteLayout>();
		}

		if (val.contains("dropShadow"))
		{
			style->dropShadow		= val["dropShadow"].get<bool>();
			style->shadowOffset.x	= val["shadowOffset"].value("x", 1);
			style->shadowOffset.y	= val["shadowOffset"].value("y", 1);
		}

		styles[key] = std::move(style);
	}
}

void UIService::realizeStyle(rmui::UIWidget& widget, float dt)
{
	auto it = styles.find(widget.style);
	if (it == styles.end() || !it->second)
	{
		SDL_Log("Style '%s' not found for widget with id %d, using default style", widget.style.c_str(), widget.id);
		return;
	}
	auto& style = *it->second;

	for (auto& [_, comp] : widget.components)
	{
		comp->realize(&widget);
	}

	Canvas2D::reset();
}

void UIService::init(int width, int height)
{
	canvasWidth	 = width;
	canvasHeight = height;

	m_root = std::make_shared<rmui::UIWidget>(idPool.next());
	m_root->rect = UIRect(0, 0, canvasWidth, canvasHeight);
	m_root->setLocalPosition(0.0f, 0.0f);
	m_root->setLocalSize(1.0f, 1.0f);
	m_root->style = "root";

	ids[m_root->id] = m_root;
	handles["root"] = m_root;
}

void UIService::resizeCanvas(int width, int height)
{
	canvasWidth = width;
	canvasHeight = height;

	m_root->rect = UIRect(0, 0, width, height);
	m_root->dirtyUpdate = true;
}

void UIService::draw(float dt)
{
	if (!m_root) 
		return;

	progressAnimations(dt);

	submissionIndex = UI_Z;
	drawRecursive(m_root.get(), m_root->rect.renderRect(), dt);
	Canvas2D::reset();
}

void UIService::update()
{
	if (m_root->dirtyUpdate)
	{
		AbsoluteLayout rootStrategy;
		updateRecursive(m_root.get(), UIRect(0, 0, canvasWidth, canvasHeight), rootStrategy, 0);
	}
}

void UIService::handleMouse(glm::vec2 mousePos)
{
	isBlocked	= false;
	focused		= nullptr;

	topWidgetAtPos(m_root.get(), mousePos);

	if (prevFocused && prevFocused != focused)
	{
		if (prevFocused->interactive)
			prevFocused->interaction->triggerOnExitHover(prevFocused);
		prevFocused = nullptr;
	}

	if (focused)
	{
		if(focused->interactive && !focused->interaction->hovered)
			focused->interaction->triggerOnEnterHover(focused);

		if (focused->interactive && ServiceLocator::get<IInputService>()->getAction("accept"))
		{
			focused->interaction->triggerOnClick(focused);
		}

		prevFocused = focused;
	}
}

void UIService::handleInput(SDL_Event& e)
{

}

void UIService::drawRecursive(rmui::UIWidget* widget, const glm::vec4 parentClip, float dt)
{
	if (!AABB(widget->rect.renderRect(), m_root->rect.renderRect()))
		return;

	if (!widget->visible) return;

	realizeStyle(*widget, dt);

	glm::vec4 prevClip = parentClip;
	glm::vec4 currentClip = parentClip;

	glm::vec4 rect = TO_GL_COORDS(widget->rect.renderRect(), m_root->rect.size.y);

	float left	 = std::max(rect.x, parentClip.x);
	float top	 = std::max(rect.y, parentClip.y);
	float right	 = std::min(rect.x + rect.z, parentClip.x + parentClip.z);
	float bottom = std::min(rect.y + rect.w, parentClip.y + parentClip.w);

	float width	 = right - left;
	float height = bottom - top;

	if (width <= 0 || height <= 0)
		return; // fully clipped

	currentClip = glm::vec4(left, top, width, height);

	Canvas2D::setIsClipping(widget->clipping);
	Canvas2D::setClipping(currentClip);
	Canvas2D::setDepth(submissionIndex++);

	for (auto& child : widget->children())
	{
		drawRecursive(child.get(), currentClip, dt);
	}

	Canvas2D::setClipping(prevClip);
}

void UIService::updateRecursive(rmui::UIWidget* widget, const UIRect& parentRect, ILayoutStrategy& strategy, int index)
{
	widget->rect = strategy.layout(*widget, parentRect, index, false);
	auto* layoutStrategy = styles[widget->style]->layoutStrategy.get();

	widget->dirtyUpdate = false;

	int childIndex = 0;
	for (auto& child : widget->children())
	{
		updateRecursive(child.get(), widget->rect, *layoutStrategy, childIndex);
		childIndex++;
	}
}

void UIService::topWidgetAtPos(rmui::UIWidget* widget, glm::vec2 mousePos)
{
	for(auto& child : widget->children())
	{
		topWidgetAtPos(child.get(), mousePos);
	}

	if (isBlocked)
		return;

	if(widget->blocking && AABB(widget->rect.renderRect(), glm::vec4(mousePos, 1 , 1)))
	{
		focused = widget;
		isBlocked = true;
	}

	return;
}

void UIService::destroy(const std::string& handle)
{
	if(!handles.contains(handle))
		return;

	handles[handle]->parent.lock()->eraseChild(handles[handle]);
	prevFocused = nullptr;

	destroy(handles[handle]);
}

void UIService::destroy(const std::shared_ptr<rmui::UIWidget>& widget)
{
	for (auto& child : widget->children())
		destroy(child);

	std::erase_if(m_animations, [widget](const auto& animation)
	{
		return animation.first == widget->id;
	});

	idPool.releaseId(widget->id);

	widget->clearChildren();
	widget->parent.reset();

	ids.erase(widget->id);
	if(handles.contains(widget->handle))
		handles.erase(widget->handle);
}

void UIService::destroy(const rmui::UIWidget* widget)
{
	destroy(widget);
}

rmui::UIWidget* const UIService::widget(int id) const
{
	if (ids.contains(id))
	{
		return ids.at(id).get();
	}

	WarnLog("UI", "Tried to extract ui widget by non-existent ID (%d)!", id);
	return nullptr;
}

rmui::UIWidget* const UIService::widget(std::string handle) const
{
	if (handles.contains(handle))
	{
		return handles.at(handle).get();
	}

	WarnLog("UI", "Tried to extract ui widget by non-existent handle (%s)!", handle.c_str());
	return nullptr;
}