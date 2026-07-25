#pragma once
#include <fstream>
#include <nlohmann/json.hpp>

#include "utility/file_util.h"

namespace core
{
	namespace fs = std::filesystem;

	struct Texts
	{
		nlohmann::json content;

		bool load(const fs::path& file)
		{
			std::ifstream f(file);
			if (!f.is_open())
				return false;
			try
			{
				content = nlohmann::json::parse(f);
			}
			catch (const nlohmann::json::parse_error&)
			{
				return false;
			}
			return true;
		}
	};

	inline Texts GTexts;
}