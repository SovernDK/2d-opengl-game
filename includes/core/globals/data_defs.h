#pragma once
#include "utility/string_interner.h"
#include <string>

constexpr const int ITEMID_EMPTY = 1;

using ItemID = StringInterner::Id;

enum class EquipSlotId { WEAPON, SIDE_WEAPON, HEAD, BODY, FEET, COUNT };
enum class AttributeId { HP, MAX_HP, STRENGTH, AGILITY, SPIRIT, BODY };

enum class ItemType { NONE, CONSUMABLE, EQUIPMENT };

struct ItemData
{
	ItemID id = 0;
	std::string name;
	ItemType type = ItemType::NONE;
	EquipSlotId eqSlotId = EquipSlotId::WEAPON;
};