#pragma once
#include "services/file_service.h"
#include "SDL3/SDL_log.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "services/service_locator.h"
#include "debug/logging.h"

#include <fstream>
#include <filesystem>

using json = nlohmann::json;

StbiImage::~StbiImage()
{
    if (data)
        stbi_image_free(data);
}

StbiImage FileService::loadFile(const std::string& path, int desiredChannel)
{
    StbiImage img;

    unsigned char* data = stbi_load(path.c_str(), &img.width, &img.height, &img.channels, desiredChannel);

    if (!data)
    {
        ErrorLog("File", "Failed to load stbi file: %s (reason: %s)", path.c_str(), stbi_failure_reason());
        img.check = false;
        return img;
    }

    img.data = data;
    img.check = true;
    return img;
}

bool FileService::savePng(const std::string& path, int width, int height, unsigned char* buffer)
{
    stbi_write_png(path.c_str(), (int)width, (int)height, 1, buffer, (int)width);

    return true;
}

json FileService::loadJsonFile(const std::string& path)
{
    try
    {
        std::ifstream file(path);
        if (!file.is_open())
            throw std::runtime_error("Cannot open file: " + path);
        return json::parse(file);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        throw std::runtime_error("Failed to parse JSON: " + path + "\n" + e.what());
    }
    catch (const std::runtime_error&)
    {
        throw; // re-throw file open error as-is
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Unexpected error loading: " + path + "\n" + e.what());
    }
}

json FileService::loadJsonFile(const std::filesystem::path path)
{
    return loadJsonFile(path.string());
}