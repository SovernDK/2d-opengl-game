#pragma once
#include "rmui/ui_widget.h"
#include "ecs/base_components.h"

class InventoryItemElement
{
	ecs::InventorySlot& slot;
	rmui::UIValueLabel& valueLabel;
};
