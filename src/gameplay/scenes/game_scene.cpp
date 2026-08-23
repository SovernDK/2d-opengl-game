#pragma once
#include "scenes/game_scene.h"

#include "SDL3/SDL.h"
#include "core/game.h"
#include "core/texts.h"
#include "core/resources.h"
#include "services/ui_service.h"

#include <scenes/character_sheet_scene.h>
#include <scenes/graph_map_scene.h>

#include "rmui/ui_widget.h"

#include <imgui.h>
#include <globals/gdata.h>

using namespace rmui;
using namespace core;

TexID tableWood;
gpu::UVRect uvs{};

void GameScene::start()
{
	IScene::start();

#pragma region Player Setup
	auto startingStats = core::GData.root["starting_stats"];

	float hp       = (float)startingStats["MaxHP"].as<int>();
	float maxHp    = (float)startingStats["MaxHP"].as<int>();
	float strength = (float)startingStats["Strength"].as<int>();
	float agility  = (float)startingStats["Agility"].as<int>();
	float spirit   = (float)startingStats["Spirit"].as<int>();
	float body     = (float)startingStats["Body"].as<int>();

	auto startingInventory = core::GData.root["starting_inventory"];
	auto tempInventory = ecs::Inventory{};

	for (const auto& entry : startingInventory)
	{
		for (const auto& kv : entry)
		{
			auto itemName = kv.first.as<std::string>();
			tempInventory.add(core::GData.interner.intern(itemName), kv.second.as<unsigned int>());
		}
	}

	m_ctx->playerEntity()
		.add<ecs::Player>({})
		.add<ecs::Stats>({
			.hp       = hp,
			.maxHp    = maxHp,
			.strength = strength,
			.agility  = agility,
			.spirit   = spirit,
			.body     = body
			})
		.add<ecs::Inventory>(tempInventory)
		.add<ecs::Equipment>({});

	auto* equipment = m_ctx->playerEntity().getMod<ecs::Equipment>();
	auto startingEquipment = core::GData.root["starting_equipment"];

	for (int i = 0; i < (int)EquipSlotId::COUNT; i++)
	{
		auto slotId = static_cast<EquipSlotId>(i);

		auto slotName = text::toLower(magic_enum::enum_name(slotId).data());
		auto itemName = startingEquipment[slotName].as<std::string>();

		equipment->equip(slotId, core::GData.interner.intern(itemName));
	}
#pragma endregion

	tableWood = Resources::loadTexture(file_util::createPath("assets", "table_wood.jpg").string(), "table_wood")->id;

	auto* left	   = Resources::loadTexture(file_util::createPath("assets", "book_left.png").string(), "book_left");
	auto* right    = Resources::loadTexture(file_util::createPath("assets", "book_right.png").string(), "book_right");
	auto* bookmark = Resources::loadTexture(file_util::createPath("assets", "bookmark.png").string(), "bookmark");

	ui = ServiceLocator::get<IUIService>();
	auto bookLeft = ui->createWindow()
		.setLocPos(.05f, .05f)
		.setLocSize(.45f, .9f)
		.setPivot(UIAnchor::Top_Left)
		.setImage(left->id)
		.setKeepAspect(true)
		.setStyle("page_left_win")
		.build("book_left_win");

	bookLeft->blocking = false;

	auto bookRight = ui->createWindow()
		.setLocPos(.95f, .05f)
		.setLocSize(.45f, .9f)
		.setPivot(UIAnchor::Top_Right)
		.setImage(right->id)
		.setKeepAspect(true)
		.setStyle("page_right_win")
		.build("book_right_win");

	auto bookmarks = ui->createWindow()
		.setLocPos(.06f, .85f)
		.setLocSize(.05f, .1f)
		.setPivot(UIAnchor::Bottom_Right)
		.setAlpha(0)
		.setStyle("bookmark_win")
		.build("bookmark");

	auto mapBookmark = ui->createButton()
		.setPivot(UIAnchor::Bottom_Right)
		.setImage(bookmark->id)
		.setKeepAspect(true)
		.setParent(bookmarks)
		.setStyle("bookmark_btn")
		.setText("M")
		.addOnClick([&](UIWidget* self)
		{
		if (currBookmark != BookmarkScene::MAP)
		{
			currBookmark = BookmarkScene::MAP;
			ServiceLocator::get<ISceneService>()->requestRemoveLast();
			ServiceLocator::get<ISceneService>()->requestTransition<GraphMapScene>(TransitionMode::Additive);
		}
		})
		.build("bookmark_map_btn");

	auto characterSheetBookmark = ui->createButton()
		.setPivot(UIAnchor::Bottom_Right)
		.setImage(bookmark->id)
		.setKeepAspect(true)
		.setParent(bookmarks)
		.setStyle("bookmark_btn")
		.setText("C")
		.addOnClick([&](UIWidget* self)
		{
		if (currBookmark != BookmarkScene::CHARACTER_SHEET)
		{
			currBookmark = BookmarkScene::CHARACTER_SHEET;
			ServiceLocator::get<ISceneService>()->requestRemoveLast();
			ServiceLocator::get<ISceneService>()->requestTransition<CharacterSheetScene>(TransitionMode::Additive);
		}
		})
		.build("bookmark_char_btn");
}

void GameScene::update(float dt)
{
	IScene::update(dt);

}

void GameScene::draw()
{
	IScene::draw();

	Canvas2D::setDepth(BACKGROUND_Z);
	Canvas2D::setColor(WHITE);
	Canvas2D::setBlend(BlendMode::None);
	Canvas2D::drawImage(tableWood, { 0, 0, 1920, 1080 }, uvs);
	Canvas2D::reset();
}

void GameScene::unload()
{
	IScene::unload();
}

void GameScene::quit()
{
	IScene::quit();
}