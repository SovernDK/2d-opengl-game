#pragma once
#include "scenes/narrative_event_scene.h"

#include "SDL3/SDL.h"

#include "core/game.h"
#include "core/texts.h"
#include "services/ui_service.h"

#include "rmui/ui_anim.h"
#include "rmui/ui_widget.h"

#include "imgui.h"
#include "scenes/loading_scene.h"
#include "graphics/rendering/canvas_2d.h"

#include "utility/file_util.h"
#include "gameplay/historia.h"
#include <magic_enum/magic_enum.hpp>

using namespace rmui;
using namespace ecs;
using namespace historia;

void NarrativeEventScene::start()
{
	IScene::start();
	
	ui = ServiceLocator::get<IUIService>();

	if(!Resources::textureExist("open_book"))
		Resources::loadTexture(core::GConfig.fromAssets("open_book.png").string(), "open_book");
	if (!Resources::textureExist("landscape"))
	{
		Resources::loadTexture(core::GConfig.fromAssets("landscape.png").string(), "landscape");
		bgTex = Resources::texture("open_book")->id;
	}

	bookWindow = ui->createWindow()
		.setLocPos(0.525f, 0.49f)
		.setLocSize(0.83f, 0.86f)
		.setPivot(UIAnchor::Center)
		.setStyle("book_window")
		.setAlpha(0)
		.build("book_window");

	leftPage = ui->createWindow()
		.setPivot(UIAnchor::Top_Left)
		.setStyle("page_window")
		.setParent(bookWindow)
		.setAlpha(0)
		.build("left_page_win");

	rightPage = ui->createWindow()
		.setPivot(UIAnchor::Top_Right)
		.setStyle("page_window")
		.setParent(bookWindow)
		.setAlpha(0)
		.build("right_page_win");

	header = ui->createImage()
		.setPivot(UIAnchor::Top_Left)
		.setStyle("page_header")
		.setParent(rightPage)
		.setImage(Resources::texture("landscape")->id)
		.build("right_page_header");

	contentUI = ui->createMultiLabel()
		.setPivot(UIAnchor::Top_Left)
		.setParent(rightPage)
		.setText(m_ctx->story()->currentContent())
		.build("right_page_content");
	
	// Choices
	choices = ui->createWindow()
		.setStyle("page_window")
		.setParent(rightPage)
		.setAlpha(0)
		.build("right_page_choices");

	int index = 0;
	for (auto& direction : m_ctx->story()->currentRoom().directions)
	{
		auto choiceBtn = ui->createButton()
			.setStyle("choice_button")
			.setParent(choices)
			.setText(magic_enum::enum_name<EDirection>(direction.dir).data())
			.setAlpha(0)
			.addOnClick([&](UIWidget* widget)
			{
				m_ctx->story()->goTo(direction.gotoId);
				updateRoom = true;
			})
			.build("choice");

		directionBtns.push_back(std::move(choiceBtn));
		index++;
	}

	for (auto& child : choices->children())
	{
		std::static_pointer_cast<UIButton>(child)->label->play<FadeIn>(0.5f);
	}
}

void NarrativeEventScene::update(float dt)
{
	IScene::update(dt);

	if (updateRoom)
	{
		contentUI->play<FadeOut>(.25f)->addListener(Listener::EXIT, [&]()
		{
			contentUI
				->tryGetComponent<UIMultilineText>()
				->setText(m_ctx->story()->currentContent());
		});
		contentUI->play<FadeIn>(.25f);

		for (auto& btn : directionBtns)
		{
			btn->label->play<FadeOut>(.25f);
		}

		m_ctx->callback(.25f, [&]()
		{
			int index = 0;
			for (auto& direction : m_ctx->story()->currentRoom().directions)
			{
				if (index < directionBtns.size())
				{
					directionBtns[index]->setText(magic_enum::enum_name<EDirection>(direction.dir).data());
					directionBtns[index]->interaction->clearOnClick();
					directionBtns[index]->interaction->addOnClick([&](UIWidget* widget)
					{
						m_ctx->story()->goTo(direction.gotoId);
						updateRoom = true;
					});
					directionBtns[index]->visible = true;
				}
				else
				{
					auto choiceBtn = ui->createButton()
						.setStyle("choice_button")
						.setParent(choices)
						.setText(magic_enum::enum_name<EDirection>(direction.dir).data())
						.setAlpha(0)
						.addOnClick([&](UIWidget* widget)
						{
							m_ctx->story()->goTo(direction.gotoId);
							updateRoom = true;
						})
						.build("choice");
					directionBtns.push_back(choiceBtn);
				}

				index++;
			}

			for (int i = index; i < directionBtns.size(); i++)
			{
				directionBtns[i]->visible = false;
			}

			choices->dirtyUpdate();

			for (auto& btn : directionBtns)
			{
				if (!btn->visible) continue;
				btn->label->play<FadeIn>(.25f);
			}
		});

		updateRoom = false;
	}
}

void NarrativeEventScene::draw()
{
	IScene::draw();

	Canvas2D::setDepth(BACKGROUND_Z);
	Canvas2D::setColor(WHITE);
	Canvas2D::setBlend(BlendMode::Alpha);
	Canvas2D::drawImage(bgTex, { 0, 0, 1920, 1080 }, uvs);
	Canvas2D::reset();
}

void NarrativeEventScene::unload()
{
	IScene::unload();

	ui->destroy("book_window");
}

void NarrativeEventScene::quit()
{
	IScene::quit();

}