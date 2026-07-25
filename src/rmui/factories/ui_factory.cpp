#pragma once
#include "rmui/factories/ui_factory.h"

#include "services/ui_service.h"
#include <services/audio_service.h>

using namespace rmui;

std::shared_ptr<UIWidget> UILabelFactory::build(const std::string& handle)
{
	auto label = service.create<UIWidget>(handle, m_style, m_parent);
	label->setLocalRect(localRect);
	label->setPivot(pivot);
	
	label->interactive = true;
	label->blocking	= false;

	const auto& style = service.style(m_style);
	//label->setAlpha(style.alpha);
	label->setAlpha(m_alpha);
	label->addComponent<UIText>(m_text, style.text);

	return label;
}

std::shared_ptr<UIWidget> UIMultiLabelFactory::build(const std::string& handle)
{
	auto label = service.create<UIWidget>(handle, m_style, m_parent);
	label->setLocalRect(localRect);
	label->setPivot(pivot);

	label->interactive = true;
	label->blocking = false;

	const auto& style = service.style(m_style);
	//label->setAlpha(style.alpha);
	label->setAlpha(m_alpha);
	label->addComponent<UIMultilineText>(m_text, style.text);

	return label;
}

std::shared_ptr<UIWidget> UIWindowFactory::build(const std::string& handle)
{
	auto window = service.create<UIWidget>(handle, m_style, m_parent);
	window->setLocalRect(localRect);
	window->setPivot(pivot);

	const auto& style = service.style(m_style);
	//window->setAlpha(style.alpha);
	window->setAlpha(m_alpha);
	window->addComponent<UIBackground>(m_image, style.back);
	window->addComponent<UIDropShadow>(style);

	return window;
}

std::shared_ptr<UIButton> UIButtonFactory::build(const std::string& handle)
{
	auto button = service.create<UIButton>(handle, m_style, m_parent);

	button->setLocalRect(localRect);
	button->setPivot(pivot);

	button->interactive = true;

	const auto& style = service.style(m_style);
	//button->setAlpha(style.alpha);
	button->setAlpha(m_alpha);
	button->addComponent<UIBackground>(m_image, style.back);
	button->addComponent<UIDropShadow>(style);

	button->interaction->addOnEnterHover([&](UIWidget* widget)
	{
		ServiceLocator::get<IAudioService>()->playOnce("clip");
	});

	if (!m_text.empty())
	{
		auto label = service.create<UIWidget>(handle + "_lbl", m_style, button);
		label->setLocalRect(UIRect());
		label->setPivot(UIAnchor::Top_Left);

		label->interactive = true;
		label->blocking = false;

		//label->setAlpha(style.alpha);
		label->setAlpha(m_alpha);
		label->addComponent<UIText>(m_text, style.text);
		button->label = label.get();

		button->interaction->addOnEnterHover([&](UIWidget* widget) { static_cast<UIButton*>(widget)->label->interaction->hovered = true; });
		button->interaction->addOnExitHover([&](UIWidget* widget) { static_cast<UIButton*>(widget)->label->interaction->hovered = false; });
	}

	if(onClick)		 button->interaction->addOnClick(onClick);
	if(onPressed)	 button->interaction->addOnPressed(onPressed);
	if(onEnterHover) button->interaction->addOnEnterHover(onEnterHover);
	if(onExitHover)	 button->interaction->addOnExitHover(onExitHover);

	return button;
}

std::shared_ptr<UIWidget> UIImageFactory::build(const std::string& handle)
{
	auto image = service.create<UIWidget>(handle, m_style, m_parent);

	image->setLocalRect(localRect);
	image->setPivot(pivot);

	image->interactive = false;
	image->blocking = true;

	const auto& style = service.style(m_style);
	//image->setAlpha(style.alpha);
	image->setAlpha(m_alpha);
	image->addComponent<UIBackground>(m_image, style.back);

	return image;
}
