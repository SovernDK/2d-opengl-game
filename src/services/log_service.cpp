#pragma once
#include "services/log_service.h"
#include "magic_enum/magic_enum.hpp"
#include "utility/file_util.h"
#include "core/config.h"

//Add counting messages so same messages dont spam logs
void LoggerService::log(LogLevel level, CategoryLevel category, std::string message)
{
    std::string levelName       = (std::string) magic_enum::enum_name(level);
    std::string categoryName    = (std::string) magic_enum::enum_name(category);

    std::string msg = std::format("[{}{}{}::{}] {}", getColorByLevel(level), levelName, TRESET, categoryName, message);
    if (!sessionLogs.empty() && sessionLogs.top().message == message)
    {
		sessionLogs.top().repeatCount++;
    }
    else
	{
		printf("%s\n", msg.c_str());
		sessionLogs.push({ getCurrentTime(), levelName, categoryName, message, 1 });
    }
}

void LoggerService::log(std::string message)
{
    log(LogLevel::Info, CategoryLevel::General, message);
}

void LoggerService::log(CategoryLevel category, std::string message) 
{ 
    log(LogLevel::Info, category, message);
}

void LoggerService::error(std::string message) 
{ 
    log(LogLevel::Error, CategoryLevel::General, message);
}

void LoggerService::error(CategoryLevel category, std::string message)
{ 
    log(LogLevel::Error, category, message);
}

void LoggerService::warn(std::string message) 
{ 
    log(LogLevel::Warn, CategoryLevel::General, message);
}

void LoggerService::warn(CategoryLevel category, std::string message) 
{ 
    log(LogLevel::Warn, category, message);
}

const char* LoggerService::getColorByLevel(LogLevel lvl)
{
    switch (lvl)
    {
    case LogLevel::Info:    return TCYAN;
    case LogLevel::Warn:    return TYELLOW;
    case LogLevel::Error:   return TRED;
    default:                return TRESET;
    }
}

std::string LoggerService::getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    return std::format("{:%H:%M:%S}", std::chrono::floor<std::chrono::milliseconds>(now));
}

void LoggerService::quit()
{
    auto now = std::chrono::system_clock::now();
    auto local = std::chrono::floor<std::chrono::seconds>(now);

    std::string time = std::format("{:%d_%m_%Y_%H_%M}", local);
    std::string fileName = time + "_session.txt";

    std::ofstream logFile(file_util::createPath(core::GConfig.logPath(), fileName));
    logFile << "TIME    LEVEL   CATEGORY    MSG     COUNT" << "\n";
    while (!sessionLogs.empty())
    {
        std::string logRow = sessionLogs.top().text();
        logFile << logRow << "\n";
        sessionLogs.pop();
    }
    logFile.close();
}