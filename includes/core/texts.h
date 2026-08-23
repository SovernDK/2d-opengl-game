#pragma once
#include <fstream>
#include <yaml-cpp/yaml.h>

#include "globals/data_defs.h"
#include "utility/file_util.h"
#include "debug/logging.h"

namespace core
{
	namespace fs = std::filesystem;

	class Texts
	{
	private:
		std::string errorMessage;
		YAML::Node root;
	public:
		bool load(const fs::path& file)
		{
			try
			{
				root = YAML::LoadFile(file.string());
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
			return true;
		}

		std::string tryGet(const std::string& key, const std::string& fallback)
		{
			if (root[key])
			{
				return root[key].as<std::string>();
			}
			
			WarnLog("Languages", "Text with key [%s] doesn't exist! Falling back to %s", key.c_str(), fallback.c_str());
			return fallback;
		}

		std::string get(const std::string& key)
		{
			if (root[key])
			{
				return root[key].as<std::string>();
			}

			ErrorLog("Languages", "Text with key [%s] doesnt exist!", key.c_str());
		}

		std::string equipmentSlotName(EquipSlotId slotId)
		{
			switch (slotId)
			{
			case EquipSlotId::WEAPON:		return tryGet("equip.slot.hand",	"Hand");
			case EquipSlotId::SIDE_WEAPON:	return tryGet("equip.slot.offhand", "Offhand");
			case EquipSlotId::HEAD:			return tryGet("equip.slot.helmet",	"Helmet");
			case EquipSlotId::BODY:			return tryGet("equip.slot.armor",	"Armor");
			case EquipSlotId::FEET:			return tryGet("equip.slot.shoes",	"Shoes");
			default:
				ErrorLog("Languages", "Unknown equipment slot id: %d", static_cast<int>(slotId));
				return "Unknown";
			}
		}

		std::string error()
		{
			return errorMessage;
		}
	};

	inline Texts GTexts;
}