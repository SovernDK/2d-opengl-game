#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "utility/file_util.h"

namespace core
{
    namespace fs = std::filesystem;
    using json = nlohmann::json;

    struct Config
    {
        std::string defaultFontName;

        fs::path assetsDir = "";
        fs::path fontsDir = "";
		fs::path shadersDir = "";
		fs::path dataDir = "";
		fs::path logsDir = "";
		fs::path textsDir = "";

		std::string defaultShader;
		std::string uiShader;
		std::string fontShader;

        bool load(const fs::path& file)
        {
            std::ifstream f(file);
            if (!f.is_open())
                return false;

            json j;
            f >> j;

            if (j.contains("defaultFontName"))
                defaultFontName = j["defaultFontName"].get<std::string>();

            if (j.contains("assetsDir"))
                assetsDir = j["assetsDir"].get<std::string>();

            if (j.contains("fontsDir"))
                fontsDir = j["fontsDir"].get<std::string>();

            if (j.contains("shadersDir"))
                shadersDir = j["shadersDir"].get<std::string>();

			if (j.contains("dataDir"))
                dataDir = j["dataDir"].get<std::string>();

            if (j.contains("logsDir"))
                logsDir = j["logsDir"].get<std::string>();

			if (j.contains("textsDir"))
                textsDir = j["textsDir"].get<std::string>();

            if (j.contains("shaders"))
            {
                auto& shaders = j["shaders"];
				defaultShader = shaders.value("default", "def");
				uiShader = shaders.value("ui", "def");
                fontShader = shaders.value("font", "def");
            }

            SDL_Log("\033[33m---- Config Loaded ----\033[0m");
            SDL_Log("\033[33m defaultFontName:\033[0m %s", defaultFontName.c_str());
            SDL_Log("\033[33m assetsDir:\033[0m %s", assetsDir.string().c_str());
            SDL_Log("\033[33m fontsDir:\033[0m %s", fontsDir.string().c_str());
			SDL_Log("\033[33m shadersDir:\033[0m %s", shadersDir.string().c_str());
			SDL_Log("\033[33m dataDir:\033[0m %s", dataDir.string().c_str());
            SDL_Log("\033[33m logsDir:\033[0m %s", logsDir.string().c_str());

            return true;
        }

        fs::path assetsPath() const
        {
            return file_util::createPath(assetsDir.string());
        }

        fs::path fontsPath() const
        {
            return file_util::createPath(assetsDir.string(), fontsDir.string());
        }

        fs::path shadersPath() const
        {
            return file_util::createPath(shadersDir.string());
        }

        fs::path logPath() const
        {
            return file_util::createPath(logsDir.string());
        }

		fs::path dataPath() const
		{
			return file_util::createPath(assetsDir.string(), dataDir.string());
		}

		fs::path textPath() const
		{
			return file_util::createPath(assetsDir.string(), textsDir.string());
		}

        fs::path defaultFontPath() const
        {
            return file_util::createPath(assetsDir.string(), fontsDir.string(), defaultFontName + ".ttf");
        }

        fs::path data(const std::string& assetName) const
        {
            return file_util::createPath(assetsDir.string(), dataDir.string(), assetName);
        }

		fs::path texts(const std::string& assetName) const
		{
			return file_util::createPath(assetsDir.string(), textsDir.string(), assetName);
		}

        fs::path fontDir(const std::string& fontName) const
        {
            return file_util::createPath(assetsDir.string(), fontsDir.string(), fontName + ".ttf");
        }

		fs::path assetDir(const std::string& asssetName, const std::string ext) const
		{
			return file_util::createPath(assetsDir.string(), asssetName + "." + ext);
		}
    };

    inline Config GConfig;
}