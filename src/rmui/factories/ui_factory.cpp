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
	label->setAlpha(m_alpha);
	label->addComponent<UIText>(m_text);

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
	label->setAlpha(m_alpha);
	label->addComponent<UIMultilineText>(m_text);

	return label;
}

std::shared_ptr<UIWidget> UIWindowFactory::build(const std::string& handle)
{
	auto window = service.create<UIWidget>(handle, m_style, m_parent);
	window->setLocalRect(localRect);
	window->setPivot(pivot);

	const auto& style = service.style(m_style);
	window->setAlpha(m_alpha);
	window->addComponent<UIBackground>(m_image);
	window->tryGetComponent<UIBackground>()->keepAspectRatio = m_keepAspectRatio;

	window->addComponent<UIDropShadow>();

	return window;
}

std::shared_ptr<UIButton> UIButtonFactory::build(const std::string& handle)
{
	auto button = service.create<UIButton>(handle, m_style, m_parent);

	button->setLocalRect(localRect);
	button->setPivot(pivot);

	button->interactive = true;

	const auto& style = service.style(m_style);
	button->setAlpha(m_alpha);
	button->addComponent<UIBackground>(m_image);
	button->addComponent<UIDropShadow>();

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
		label->blocking	   = false;

		label->setAlpha(m_alpha);
		label->addComponent<UIText>(m_text);
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
	image->setAlpha(m_alpha);
	image->addComponent<UIBackground>(m_image);

	return image;
}

// Remember to add forward declarations in ui_widget.h
std::shared_ptr<UIValueLabel> UIValueLabelFactory::build(const std::string& handle)
{
	auto valueLabelWindow = service.create<UIValueLabel>(handle, m_style, m_parent);
	valueLabelWindow->setLocalRect(localRect);
	valueLabelWindow->setPivot(pivot);

	valueLabelWindow->interactive = true;
	valueLabelWindow->blocking = true;

	valueLabelWindow->setAlpha(m_alpha);
	valueLabelWindow->addComponent<UIBackground>(m_image);

#pragma region Name Label
	auto textLabel = service.create<UIWidget>(handle+"_n", m_style, valueLabelWindow);
	textLabel->setLocalRect(UIRect{ 0.0f, 0.0f, 1.0f, 1.0f });
	textLabel->setPivot(pivot);

	textLabel->interactive = true;
	textLabel->blocking = false;

	textLabel->setAlpha(m_alpha);
	textLabel->addComponent<UIText>(m_text);
	valueLabelWindow->namelabel = textLabel.get();
#pragma endregion

#pragma region Value Label
	auto valueLabel = service.create<UIWidget>(handle+"_v", m_valueStyle, valueLabelWindow);
	valueLabel->setLocalRect(UIRect{ 0.0f, 0.0f, 1.0f, 1.0f });
	valueLabel->setPivot(pivot);

	valueLabel->interactive = true;
	valueLabel->blocking = false;

	valueLabel->setAlpha(m_alpha);
	valueLabel->addComponent<UIText>(m_value);

	if (onClick) valueLabelWindow->interaction->addOnClick(onClick);

	valueLabelWindow->valueLabel = valueLabel.get();
#pragma endregion

	valueLabelWindow->interaction->addOnEnterHover([&](UIWidget* widget) 
	{ 
		static_cast<UIValueLabel*>(widget)->namelabel->interaction->hovered = true; 
		static_cast<UIValueLabel*>(widget)->valueLabel->interaction->hovered = true;
	});
	valueLabelWindow->interaction->addOnExitHover([&](UIWidget* widget) 
	{ 
		static_cast<UIValueLabel*>(widget)->namelabel->interaction->hovered = false;
		static_cast<UIValueLabel*>(widget)->valueLabel->interaction->hovered = false;
	});

	return valueLabelWindow;
}