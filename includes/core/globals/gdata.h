#pragma once
#include <fstream>
#include <yaml-cpp/yaml.h>

#include "utility/file_util.h"
#include "debug/logging.h"
#include "memory/flat_map.h"

#include "globals/data_defs.h"
#include <magic_enum/magic_enum.hpp>

namespace core
{
	namespace fs = std::filesystem;

	class GameData
	{
	private:
		std::string errorMessage;
	public:
		YAML::Node root;
		mem::flat_map<ItemID, ItemData> items;
		StringInterner interner;
	public:
		bool load(const fs::path& file)
		{
			try
			{
				root = YAML::LoadFile(file.string());

				for(const auto& entry : root["items"])
				{
					for (const auto& kv : entry)
					{
						ItemID id = interner.intern(kv.first.as<std::string>());

						EquipSlotId eqSlotId = EquipSlotId::WEAPON;
						auto eqSlotNode = kv.second["eq_slot"];
						if (eqSlotNode && eqSlotNode.IsDefined() && !eqSlotNode.IsNull())
						{
							auto eqSlotStr = eqSlotNode.as<std::string>();
							eqSlotId = magic_enum::enum_cast<EquipSlotId>(eqSlotStr, 
								magic_enum::case_insensitive)
								.value_or(EquipSlotId::WEAPON);
						}

						auto itemType = magic_enum::enum_cast<ItemType>(kv.second["type"].as<std::string>(), 
							magic_enum::case_insensitive)
							.value_or(ItemType::NONE);

						ItemData data
						{ 
							.id = id, 
							.name = kv.second["name"].as<std::string>(), 
							.type = itemType,
							.eqSlotId = eqSlotId
						};
						items.insert(id, data);
					}
				}
			}
			catch (const YAML::BadFile& e)
			{
				errorMessage = std::string("Failed to open YAML file: ") + e.what();
				return false;
			}
			catch (const YAML::ParserException& e)
			{
				errorMessage = std::string("Failed to parse YAML file: ") + e.what();
				return false;
			}
			catch (const YAML::Exception& e)
			{
				errorMessage = std::string("YAML error: ") + e.what();
				return false;
			}

			InfoLog("GameData", "Successfully loaded game data from file: %s", file.string().c_str());
			return true;
		}

		template<typename TType>
		std::string tryGet(const std::string& key, const std::string& fallback) const
		{
			if (root[key])
			{
				return root[key].as<TType>();
			}

			WarnLog("GameData", "Game Data with key [%s] doesn't exist! Falling back to %s", key.c_str(), fallback.c_str());
			return fallback;
		}

		template<typename TType>
		std::string get(const std::string& key) const
		{
			if (root[key])
			{
				return root[key].as<TType>();
			}

			ErrorLog("GameData", "Game Data with key [%s] doesnt exist!", key.c_str());
		}
		
		const std::string& itemName(ItemID id) const
		{
			if (items.contains(id))
			{
				return items.at(id).name;
			}

			ErrorLog("GameData", "Item with ID [%d] doesn't exist!", id);
			static const std::string emptyString;
			return emptyString;
		}

		std::string itemName(const std::string& name)
		{
			ItemID id = interner.intern(name);
			return itemName(id);
		}

		std::string error()
		{
			return errorMessage;
		}
	};

	inline GameData GData;
}