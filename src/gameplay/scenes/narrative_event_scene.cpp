#pragma once
#include "scenes/narrative_event_scene.h"

#include "SDL3/SDL.h"

#include "ecs/base_components.h"

#include "core/game.h"
#include "core/texts.h"
#include "core/globals/gdata.h"
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

	auto rightPageWin = ui->widget("book_right_win");
	if(rightPageWin.expired())
	{
		ErrorLog("NarrativeEventScene", "Failed to find right page window!");
		return;
	}

	header = ui->createLabel()
		.setLocPos(UIAnchor::Top_Left)
		.setPivot(UIAnchor::Top_Left)
		.setLocSize(1.0f, .1f)
		.setParent(rightPageWin.lock())
		.setText("")
		.setStyle("page_header")
		.build("right_page_header");

	contentUI = ui->createMultiLabel()
		.setLocPos(UIAnchor::Top_Left)
		.setPivot(UIAnchor::Top_Left)
		.setLocSize(1.0f, .6f)
		.setParent(rightPageWin.lock())
		.setText("")
		.setStyle("page_content")
		.build("right_page_content");
	
	// Choices
	actions = ui->createWindow()
		.setLocPos(UIAnchor::Top_Left)
		.setPivot(UIAnchor::Top_Left)
		.setLocSize(1.0f, .3f)
		.setParent(rightPageWin.lock())
		.setAlpha(0)
		.setStyle("rpage_actions")
		.build("right_page_actions");

	choices = ui->createWindow()
		.setLocPos(UIAnchor::Top_Left)
		.setPivot(UIAnchor::Top_Left)
		.setLocSize(.6f, 1.0f)
		.setParent(actions)
		.setAlpha(0)
		.setStyle("rpage_choices")
		.build("right_page_choices");

	directions = ui->createWindow()
		.setLocPos(UIAnchor::Top_Left)
		.setPivot(UIAnchor::Top_Left)
		.setLocSize(.4f, 1.0f)
		.setParent(actions)
		.setAlpha(0)
		.setStyle("rpage_directions")
		.build("right_page_directions");

	// Direction buttons
	auto directionBtn = ui->createButton()
		.setLocPos(UIAnchor::Top_Left)
		.setPivot(UIAnchor::Top_Left)
		.setLocSize(1.0f, .25f)
		.setParent(directions)
		.setAlpha(0)
		.setStyle("page_choice_btn");

	auto north = directionBtn
		.setText(core::GTexts.get("paragraph.north"))
		.build("btn_north");

	auto west = directionBtn
		.setText(core::GTexts.get("paragraph.west"))
		.build("btn_west");

	auto east = directionBtn
		.setText(core::GTexts.get("paragraph.east"))
		.build("btn_east");

	auto south = directionBtn
		.setText(core::GTexts.get("paragraph.south"))
		.build("btn_south");

	directionBtns[EDirection::NORTH] = north;
	directionBtns[EDirection::WEST]  = west;
	directionBtns[EDirection::EAST]  = east;
	directionBtns[EDirection::SOUTH] = south;

	directionBtns[EDirection::NORTH]->label->play<FadeIn>(.25f);
	directionBtns[EDirection::WEST]->label->play<FadeIn>(.25f);
	directionBtns[EDirection::EAST]->label->play<FadeIn>(.25f);
	directionBtns[EDirection::SOUTH]->label->play<FadeIn>(.25f);

	updateRoom = true;
}

void NarrativeEventScene::update(float dt)
{
	IScene::update(dt);

	if (updateRoom)
	{
		header->play<FadeOut>(.25f)->addListener(Listener::EXIT, [&]()
		{
			header
				->tryGetComponent<UIText>()
				->text = m_ctx->story()->currentHeader();
		});
		header->play<FadeIn>(.25f);

		contentUI->play<FadeOut>(.25f)->addListener(Listener::EXIT, [&]()
		{
			contentUI
				->tryGetComponent<UIMultilineText>()
				->setText(m_ctx->story()->currentContent());
		});
		contentUI->play<FadeIn>(.25f);

		for (auto& btn : choiceBtns)
		{
			btn->label->play<FadeOut>(.25f);
		}

		// Choices
		m_ctx->callback(.25f, [&]()
		{
			int index = 0;
			for (auto& choice : m_ctx->story()->currParagraph().choices)
			{
				if (index < choiceBtns.size())
				{
					choiceBtns[index]->setText(choice.content);
					choiceBtns[index]->visible = true;
					choiceBtns[index]->interaction->setOnClick([&, ch = choice](UIWidget* self)
					{
						for (const auto& eff : ch.effects)
						{
							applyEffect(eff);
						}
						m_ctx->story()->goTo(ch.gotoId);
						updateRoom = true;
					});
				}
				else
				{
					auto choiceBtn = ui->createButton()
						.setLocPos(UIAnchor::Top_Left)
						.setPivot(UIAnchor::Top_Left)
						.setLocSize(1.0f, .25f)
						.setParent(choices)
						.setText(choice.content)
						.setAlpha(0)
						.setStyle("page_choice_btn")
						.addOnClick([&, ch = choice](UIWidget* self)
						{
							for (const auto& eff : ch.effects)
							{
								applyEffect(eff);
							}
							m_ctx->story()->goTo(ch.gotoId);
							updateRoom = true;
						})
						.build(std::format("choice_{}", index));
					choiceBtns.push_back(choiceBtn);
				}

				index++;
			}

			for (int i = index; i < choiceBtns.size(); i++)
			{
				choiceBtns[i]->visible = false;
			}

			choices->dirtyUpdate();

			for (auto& btn : choiceBtns)
			{
				if (!btn->visible) continue;
				btn->label->play<FadeIn>(.25f);
			}
		});

		// Directions
		auto& room = m_ctx->story()->currentRoom();
		for (int i = 0; i < (int) EDirection::COUNT; i++)
		{
			auto dir = (EDirection) i;
			if (room.directions.contains(dir))
			{
				directionBtns[dir]->setInteractive(true);
				directionBtns[dir]->interaction->setOnClick([&, direction = dir](UIWidget* widget)
				{
					auto id = room.directions.at(direction).gotoId;
					m_ctx->story()->move(id);
					updateRoom = true;
				});
			}
			else
			{
				directionBtns[dir]->interaction->clearOnClick();
				directionBtns[dir]->setInteractive(false);
			}
		}

		updateRoom = false;
	}
}

void NarrativeEventScene::draw()
{
	IScene::draw();
}

void NarrativeEventScene::unload()
{
	IScene::unload();

	header.reset();
	contentUI.reset();
	choices.reset();

	directionBtns.clear();
}

void NarrativeEventScene::quit()
{
	IScene::quit();
}

void NarrativeEventScene::applyEffect(const ChoiceEffect& effect)
{
	switch (effect.type)
	{
	case EEffectType::GIVE_ITEM:
		auto itemId = core::GData.interner.intern(effect.params.at(YAML_PAR_GIVE_ITEM));
		m_ctx->playerEntity().getMod<ecs::Inventory>()->add(itemId, 1);
		break;
	}
}