#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "memory/flat_map.h"
#include "graphics/material.h"

#include "core/time/timer.h"
#include "globals/data_defs.h"

#include "utility/gradient.h"
#include "utility/feel.h"
#include "utility/string_interner.h"

#include "debug/logging.h"
#include "resources.h"

#include <config.h>
#include <magic_enum/magic_enum.hpp>

using Depth = int;

namespace ecs
{
	struct Transform
	{
		glm::vec3 position  = glm::vec3(0.0f);
		glm::vec3 rotation  = glm::vec3(0.0f);
		glm::vec3 scale     = glm::vec3(1.0f);
	};

	struct Transform2D
	{
		glm::vec2 position  = glm::vec2(0.0f);
		glm::vec2 scale     = glm::vec2(1.0f);
		float rotation      = 0.0f;

		glm::mat4 model(glm::vec2 size) const
		{
			glm::vec3 pos = glm::vec3(position, 1.0f);

			float scaledSizeX = scale.x * size.x;
			float scaledSizeY = scale.y * size.y;

			glm::mat4 model = glm::mat4(1.0f);

			// translate
			model = glm::translate(model, pos);

			//rotate around center
			model = glm::translate(model, glm::vec3(0.5f * scaledSizeX, 0.5f * scaledSizeY, 0.0f));
			model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::translate(model, glm::vec3(-0.5f * scaledSizeX, -0.5f * scaledSizeY, 0.0f));

			// scale
			model = glm::scale(model, glm::vec3(scaledSizeX, scaledSizeY, 1));

			return model;
		}
		glm::mat4 model(float w, float h) const
		{
			return model(glm::vec2(w, h));
		}
	};

	struct Sprite
	{
		TexID texture{ 0 };
		BlendMode blend = BlendMode::None;

		gpu::UVRect uv{};
		glm::vec2 size = glm::vec2(32);
		SDL_Color color = WHITE;
		Depth depth = 1;
		bool isClipping = false;
		glm::vec4 clipping{};

		MaterialInstance material = MaterialInstance(Resources::sharedMat(core::GConfig.shaders.def));
	};

	struct Particle
	{
		Timer lifeTime{ 1.0f };
		glm::vec2 velocity{ 0.0f };
		glm::vec2 scale{ 1.0f };
		uint8_t alpha = 255;
		SDL_Color color = WHITE;

		feel::ICurve* velCurve = nullptr;
		feel::ICurve* scaleCurve = nullptr;
		feel::ICurve* alphaCurve = nullptr;
		feel::ICurve* colCurve = nullptr;
	};

	struct ParticleEmitter
	{
		Timer emiting{ 0.0f };
		Timer interval{ 0.0f };

		TexID texture{ 0 };

		glm::vec4 emitArea{ 0.0f, 0.0f, 100.0f, 200.0f };

		// Starting
		glm::vec2 size = glm::vec2(64.0f);
		glm::vec2 lifeRange{ 1.0f };
		glm::vec2 startVelocity{ 1.0f };
		glm::vec2 startScale{ 1.0f };
		uint8_t startAlpha = 255;
		SDL_Color startColor = WHITE;
		glm::vec2 direction{ 1.0f };

		// Velocity
		std::unique_ptr<feel::ICurve> velCurve = std::make_unique<feel::QuadCurve>();
		// Scale
		std::unique_ptr<feel::ICurve> scaleCurve = std::make_unique<feel::QuadCurve>();
		// Alpha
		std::unique_ptr<feel::ICurve> alphaCurve = std::make_unique<feel::QuadCurve>();
	};

	struct MapGenSettings
	{
		float frequency = 0.001f;
		int octaves = 8;
		float gain = 0.5f;
		int seed = 1337;

		int width = 1920;
		int height = 1080;

		float sunX = 1.1f;
		float sunY = 1.1f;
		float sunZ = 2.0f;

		glm::vec3 ambientColor = glm::vec3(1.0f, 0.9f, 0.9f);
		float ambientStrength = 0.1f;
		float diffuseAmbient = 0.25f;
		float specularStrength = 0.3f;

		int steps = 64;
		float stepSize = 0.001f;
		float shadowLength = 0.015f;
		float shadowStr = 0.7f;
		float waterShadowStr = 0.2f;

		// Terrain
		float terrainSpecStr = 0.2f;
		float terrainSpecSpred = 32.0f;
		float terrainNormStr = 0.2f;

		// Forest
		float forestBumpStr = 0.5f;
		float forestNormStr = 0.3f;

		// Water
		glm::vec3 waterColor = glm::vec3(0.38f, 0.65f, 0.66f);
		glm::vec3 deepWaterColor = glm::vec3(0.05f, 0.18f, 0.40f);

		float waterSpecStr = 0.45f;
		float waterNormalStr = 0.5f;
		float waterLevel = 0.36f;
		float waterSpecSpread = 64.0f;
	};

	struct WorldMap
	{

	};

	struct Player
	{
		std::string name = "Player";
	};

	//Add wounds system which gives different negative effects
	//Getting 3 wounds = death?
	struct Wound
	{

	};

	struct InventorySlot
	{
		ItemID itemId;
		unsigned int quantity = 0;
	};

	struct Inventory
	{
		bool updated = false;
		unsigned int cap = 10;
		std::vector<InventorySlot> items;

		void add(ItemID itemId, unsigned int quantity)
		{
			updated = true;

			for (auto& slot : items)
			{
				if (slot.itemId == itemId)
				{
					slot.quantity += quantity;
					return;
				}
			}

			if (items.size() < cap)
			{
				items.push_back({ itemId, quantity });
			}
			else
			{
				ErrorLog("Inventory", "Cannot add item [ID=%d] to inventory. Inventory is full!", itemId);
			}
		}

		void subtract(int slotId, unsigned int quantity)
		{
			updated = true;

			if (slotId < 0 || slotId >= items.size())
			{
				ErrorLog("Inventory", "Cannot subtract item from inventory. Invalid slot ID: %d", slotId);
				return;
			}

			items[slotId].quantity -= quantity;
			if(items[slotId].quantity < 1)
			{
				items.erase(items.begin() + slotId);
			}
		}
			
		void remove(int slotId)
		{
			updated = true;

			if (slotId < 0 || slotId >= items.size())
			{
				ErrorLog("Inventory", "Cannot remove item from inventory. Invalid slot ID: %d", slotId);
				return;
			}
			items.erase(items.begin() + slotId);
		}
	};

	struct EquipmentSlot
	{
		EquipSlotId id;
		ItemID itemId = ITEMID_EMPTY;
	};

	struct Equipment
	{
		std::vector<EquipmentSlot> slots
		{
			{
				{ EquipSlotId::WEAPON },
				{ EquipSlotId::SIDE_WEAPON },
				{ EquipSlotId::HEAD },
				{ EquipSlotId::BODY },
				{ EquipSlotId::FEET }
			}
		};

		const EquipmentSlot& slot(EquipSlotId slotId) const
		{
			for (auto& slot : slots)
			{
				if (slot.id == slotId)
					return slot;
			}

			auto slotName = magic_enum::enum_name(slotId);
			ErrorLog("Equipment", "Slot %s not found!", slotName.data());
			throw std::runtime_error("Slot not found");
		}

		void equip(EquipSlotId slotId, ItemID itemId)
		{
			for (auto& slot : slots)
			{
				if (slot.id == slotId)
				{
					slot.itemId = itemId;
				}
			}
		}

		void unequip(EquipSlotId slotId)
		{
			for (auto& slot : slots)
			{
				if (slot.id == slotId)
				{
					slot.itemId = ITEMID_EMPTY;
				}
			}
		}
	};

	struct Attribute
	{
		float value = 1.0f;
	};

	struct Stats
	{
		Attribute hp;
		Attribute maxHp;

		const float minValue = 1.0f;
		const float maxValue = 100.0f;

		Attribute strength;
		Attribute agility;
		Attribute spirit;
		Attribute body;
	};

	struct Enemy
	{
		std::string name = "Enemy";
	};

	// Actions
	// Inventory/Equipment actions
	struct InvUseItemAction
	{
		int invSlotId;
		ItemID id;
		ItemType type;
	};

	struct AddItemAction
	{
		ItemID itemId;
		unsigned int quantity = 1;
	};

	struct SubtractItemAction
	{
		int invSlotId;
		unsigned int quantity = 1;
	};

	struct EquipAction
	{
		EquipSlotId slotId;
		ItemID itemId;
	};

	struct InvEquipAction
	{
		int inventoryId;
		EquipSlotId slotId;
	};

	struct UnequipAction
	{
		EquipSlotId slotId;
	};

	// Graph map
	struct MapNode
	{

	};
}