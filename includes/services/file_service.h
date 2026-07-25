#pragma once
#include <string>
#include <stb_image.h>

#include "services/service.h"
#include <nlohmann/json.hpp>

struct StbiImage
{
    bool check = false;
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;

    StbiImage() = default;
    ~StbiImage();

    StbiImage(const StbiImage&) = delete;
    StbiImage& operator=(const StbiImage&) = delete;

    StbiImage(StbiImage&& o) noexcept
        : data(o.data), width(o.width), height(o.height), channels(o.channels)
    {
        o.data = nullptr;
    }
};

class IFileService : IService
{
public:
    IFileService() = default;
    IFileService(const IFileService&) = delete;
    IFileService& operator=(const IFileService&) = delete;
	IFileService(IFileService&&) = default;
	IFileService& operator=(IFileService&&) = default;

    virtual ~IFileService() = default;

	virtual StbiImage loadFile(const std::string& path, int channel) = 0;
	virtual bool savePng(const std::string& path, int width, int height, unsigned char* buffer) = 0;
    virtual nlohmann::json loadJsonFile(const std::string& path) = 0;
    virtual nlohmann::json loadJsonFile(const std::filesystem::path path) = 0;
};

class FileService : public IFileService
{
public:
    StbiImage loadFile(const std::string& path, int channel) override;
    bool savePng(const std::string& path, int width, int height, unsigned char* buffer) override;
    nlohmann::json loadJsonFile(const std::string& path) override;
    nlohmann::json loadJsonFile(const std::filesystem::path path) override;
};