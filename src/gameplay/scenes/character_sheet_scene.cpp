#pragma once
#include "scenes/character_sheet_scene.h"

#include "SDL3/SDL.h"
#include "core/game.h"
#include "core/texts.h"
#include "services/ui_service.h"

#include "rmui/ui_widget.h"

#include <imgui.h>
#include <globals/gdata.h>
#include <magic_enum/magic_enum.hpp>

using namespace rmui;
using namespace core;

std::shared_ptr<UIWidget> leftPageWindow = nullptr;

void CharacterSheetScene::start()
{
	ui = ServiceLocator::get<IUIService>();
	auto& playerEntity = m_ctx->playerEntity();
	
	auto leftPageWindow = ui->widget("book_left_win");
	if (leftPageWindow.expired())
	{
		ErrorLog("CharacterSheetScene", "Failed to find left page window!");
		return;
	}
	
	//left column
	auto leftColumn = ui->createWindow()
		.setLocPos(UIAnchor::Top_Left)
		.setPivot(UIAnchor::Top_Left)
		.setLocSize(1.0f, 1.0f)
		.setParent(leftPageWindow.lock())
		.setStyle("lpage_column")
		.setAlpha(0)
		.build("left_page_columns");

#pragma region Attributes
	auto attributesTitle = ui->createLabel()
		.setLocPos(UIAnchor::Top_Left)
		.setLocSize(1.0f, 0.05f)
		.setPivot(UIAnchor::Top_Left)
		.setParent(leftColumn)
		.setText(GTexts.get("stats.title"))
		.build("stats_title_lbl");

	auto attributes = ui->createWindow()
		.setLocPos(UIAnchor::Top_Left)
		.setLocSize(1.0f, .35f)
		.setPivot(UIAnchor::Top_Left)
		.setParent(leftColumn)
		.setStyle("lpage_attributes")
		.setAlpha(0)
		.build("player_stats_win");

	auto attributeValueLabel = ui->createValueLabel()
		.setLocPos(UIAnchor::Top_Left)
		.setPivot(UIAnchor::Top_Left)
		.setLocSize(1.0f, .2f)
		.setValueTextStyle("value_label")
		.setAlpha(0)
		.setParent(attributes);

	const auto* playerStats = playerEntity.get<ecs::Stats>();
	auto hpLabel = attributeValueLabel
		.setValue(text::tostr(playerStats->maxHp.value))
		.setText(GTexts.get("stats.hp")).build("hp_lbl");

	auto strengthLabel = attributeValueLabel
		.setValue(text::tostr(playerStats->strength.value))
		.setText(GTexts.get("stats.strength")).build("strength_lbl");

	auto agilityLabel = attributeValueLabel
		.setValue(text::tostr(playerStats->agility.value))
		.setText(GTexts.get("stats.agility")).build("agility_lbl");

	auto spiritLabel = attributeValueLabel
		.setValue(text::tostr(playerStats->spirit.value))
		.setText(GTexts.get("stats.spirit")).build("spirit_lbl");

	auto bodyLabel = attributeValueLabel
		.setValue(text::tostr(playerStats->body.value))
		.setText(GTexts.get("stats.body")).build("body_lbl");

	int index = 0;
	for (auto& child : attributes->children())
	{
		auto ptr = std::static_pointer_cast<UIValueLabel>(child);
		ptr->namelabel->play<FadeIn>(.5f);
		ptr->valueLabel->play<FadeIn>(.5f);
	}
#pragma endregion

#pragma region Equipment Slots
	const auto* playerEquipment = playerEntity.get<ecs::Equipment>();

	auto equipmentTitle = ui->createLabel()
		.setLocPos(UIAnchor::Top_Left)
		.setLocSize(1.0f, .05f)
		.setPivot(UIAnchor::Top_Left)
		.setParent(leftColumn)
		.setText(GTexts.get("equip.title"))
		.build("equip_title_lbl");

	equipment = ui->createWindow()
		.setLocPos(UIAnchor::Top_Left)
		.setLocSize(1.0f, .35f)
		.setPivot(UIAnchor::Top_Left)
		.setParent(leftColumn)
		.setStyle("lpage_equipment")
		.setAlpha(0)
		.build("player_equip_win");

	auto equipmentValueLabel = ui->createValueLabel()
		.setLocPos(UIAnchor::Top_Left)
		.setLocSize(1.0f, 1.0f)
		.setPivot(UIAnchor::Top_Left)
		.setAlpha(0)
		.setValueTextStyle("value_label")
		.setParent(equipment);

	index = 0;
	for(auto& slot : playerEquipment->slots)
	{
		auto slotName = magic_enum::enum_name(slot.id);
		auto slotLabel = equipmentValueLabel
			.addOnClick([&, slotId = index](UIWidget* self)
			{
				auto& playerEntity = m_ctx->playerEntity();
				auto* selfValueLabel = static_cast<UIValueLabel*>(self);
				const auto itemId = playerEntity.get<ecs::Equipment>()->slot((EquipSlotId) slotId).itemId;
				if (itemId == ITEMID_EMPTY) return;

				playerEntity.add<ecs::AddItemAction>({ itemId, 1 });
				playerEntity.add<ecs::UnequipAction>({ (EquipSlotId) slotId });
			})
			.setValue(GData.itemName(slot.itemId))
			.setText(GTexts.equipmentSlotName(slot.id))
			.build(std::format("slot_{}_lbl", slotName));
		slotLabel->namelabel->play<FadeIn>(.5f);
		slotLabel->valueLabel->play<FadeIn>(.5f);

		index++;
	}
#pragma endregion

#pragma region Inventory
	rightColumn = ui->createWindow()
		.setLocPos(UIAnchor::Top_Left)
		.setLocSize(1.0f, 1.0f)
		.setPivot(UIAnchor::Top_Left)
		.setParent(leftPageWindow.lock())
		.setStyle("lpage_column")
		.setAlpha(0)
		.build("lpage_rcolumn");

	auto inventoryTitle = ui->createLabel()
		.setLocPos(UIAnchor::Top_Left)
		.setLocSize(1.0f, .05f)
		.setPivot(UIAnchor::Top_Left)
		.setParent(rightColumn)
		.setText(GTexts.get("inventory.title"))
		.build("inv_title_lbl");

	inventoryWin = ui->createWindow()
		.setLocPos(UIAnchor::Top_Left)
		.setLocSize(1.0f, .95f)
		.setPivot(UIAnchor::Top_Left)
		.setParent(rightColumn)
		.setStyle("lpage_inventory")
		.setAlpha(0)
		.build("inventory_window");

	m_ctx->playerEntity().getMod<ecs::Inventory>()->updated = true;
#pragma endregion
}

void CharacterSheetScene::update(float dt)
{
	bool invUpdated = m_ctx->playerEntity().get<ecs::Inventory>()->updated;
	if (invUpdated)
	{
		// Inventory update
		for (auto& child : inventoryWin->children())
		{
			child->visible = false;
		}

		const auto& items = m_ctx->playerEntity().get<ecs::Inventory>()->items;
		for (int i = 0; i < items.size(); i++)
		{
			if (i < inventoryWin->children().size())
			{
				auto handle = std::format("inv_slot_lbl_{}", i);
				auto slotLabel = std::static_pointer_cast<UIValueLabel>(inventoryWin->children()[i]);
				auto& itemName = GData.itemName(items[i].itemId);

				slotLabel->setValue(text::tostr(items[i].quantity));
				slotLabel->setText(GTexts.get(itemName));
				slotLabel->visible = true;
			}
			else
			{
				auto handle = std::format("inv_slot_lbl_{}", i);
				auto itemValueLabel = ui->createValueLabel()
					.setLocPos(UIAnchor::Top_Left)
					.setLocSize(1.0f, 0.05f)
					.setPivot(UIAnchor::Top_Left)
					.setAlpha(0)
					.setValueTextStyle("value_label_numbers")
					.setParent(inventoryWin)
					.addOnClick([&, slotId = i](UIWidget* self)
					{
						auto& items = m_ctx->playerEntity().getMod<ecs::Inventory>()->items;
						m_ctx->playerEntity().add<ecs::InvUseItemAction>({ 
							.invSlotId = slotId,
							.id = items[slotId].itemId,
							.type = GData.items[items[slotId].itemId].type
							});
					})
					.setValue(text::tostr(items[i].quantity))
					.setText(GTexts.get(GData.itemName(items[i].itemId)))
					.build(handle);

				itemValueLabel->namelabel->play<FadeIn>(.5f);
				itemValueLabel->valueLabel->play<FadeIn>(.5f);
			}
		}

		m_ctx->playerEntity().getMod<ecs::Inventory>()->updated = false;
	}
	
	// Equipment update
	const auto* playerEquipment = m_ctx->playerEntity().get<ecs::Equipment>();
	int index = 0;
	for (auto& slot : playerEquipment->slots)
	{
		auto& itemName = GData.itemName(slot.itemId);

		static_cast<UIValueLabel*>(equipment->children()[index].get())->setValue(GTexts.get(itemName));
		index++;
	}
}

void CharacterSheetScene::draw()
{
	
}

void CharacterSheetScene::unload()
{
	auto* ui = ServiceLocator::get<IUIService>();

	ui->destroy("left_page_columns");
	ui->destroy("lpage_rcolumn");

	leftPageWindow.reset();
	rightColumn.reset();
	inventoryWin.reset();
	equipment.reset();
	inventoryElements.clear();
}

void CharacterSheetScene::quit()
{
	
}