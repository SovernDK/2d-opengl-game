#pragma once
#include <string>
#include <filesystem>
#include <cstdio>
#include <cstdarg>

#include "utility/file_util.h"

#define InfoLog(tag, fmt, ...)			::Debug::Log(::Debug::LogLevel::Info, tag, ::Debug::formatMessage(fmt, ##__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#define WarnLog(tag, fmt, ...)			::Debug::Warn(tag, ::Debug::formatMessage(fmt, ##__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#define ErrorLog(tag, fmt, ...)			::Debug::Error(tag, ::Debug::formatMessage(fmt, ##__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)
#define FatalErrorLog(tag, fmt, ...)	::Debug::FatalError(tag, ::Debug::formatMessage(fmt, ##__VA_ARGS__), __FILE__, __FUNCTION__, __LINE__)

namespace Debug
{
	enum class LogLevel
	{
		Info, Warn, Error, FatalError
	};

	constexpr const char* TRESET = "\033[0m";
	constexpr const char* TRED = "\033[31m";
	constexpr const char* TGREEN = "\033[32m";
	constexpr const char* TYELLOW = "\033[33m";
	constexpr const char* TBLUE = "\033[34m";
	constexpr const char* TMAGENTA = "\033[35m";
	constexpr const char* TCYAN = "\033[36m";
	constexpr const char* TWHITE = "\033[37m";

	inline file_util::fs::path LOG_PATH;

#pragma region Helper functions
	inline std::string formatMessage(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		va_list argsCopy;
		va_copy(argsCopy, args);
		int size = std::vsnprintf(nullptr, 0, fmt, argsCopy);
		va_end(argsCopy);

		if (size <= 0)
		{
			va_end(args);
			return std::string();
		}

		std::string result(static_cast<size_t>(size), '\0');
		std::vsnprintf(result.data(), static_cast<size_t>(size) + 1, fmt, args);
		va_end(args);

		return result;
	}

	inline std::string getCurrentTime()
	{
		auto now = std::chrono::system_clock::now();
		return std::format("{:%H:%M:%S}", std::chrono::floor<std::chrono::milliseconds>(now));
	}

	inline void saveToFile(const std::string& msg)
	{
		auto now = std::chrono::system_clock::now();
		auto local = std::chrono::floor<std::chrono::seconds>(now);

		std::string time = std::format("{:%d_%m_%Y}", local);
		std::string fileName = time + "_session.txt";

		if (!LOG_PATH.empty())
		{
			auto path = file_util::createPath(LOG_PATH, fileName);
			std::ofstream logFile(path, std::ios::app);
			if (logFile.is_open())
			{
				logFile << msg << std::endl;
				logFile.close();
			}
		}
	}

	inline const char* getColorByLevel(LogLevel lvl)
	{
		switch (lvl)
		{
		case LogLevel::Info:    return TCYAN;
		case LogLevel::Warn:    return TYELLOW;
		case LogLevel::Error:   return TRED;
		default:                return TRESET;
		}
	}
#pragma endregion Helper functions
	
#pragma region Logging
	inline void Init(const file_util::fs::path& logPath)
	{
		LOG_PATH = logPath;

		saveToFile("=========== SESSION STARTED ===========");
	}

	inline void Destroy()
	{
		saveToFile("=========== SESSION ENDED ===========");
	}

	inline void Log(LogLevel level, const std::string& tag, const std::string& msg,
		const char* file, const char* funcName, unsigned int lineNum)
	{
		auto time = getCurrentTime();
		std::string formatedMsg = std::format("[{}{}::{}{}] {}", getColorByLevel(level), time, tag, TRESET, msg);
		printf("%s\n", formatedMsg.c_str());

		std::string fileMessage = std::format("[{}::{}] {} - {} {} {}", time, tag, msg, file, funcName, lineNum);
		saveToFile(fileMessage);
	}

	inline void Warn(const std::string& tag, const std::string& msg,
		const char* file, const char* funcName, unsigned int lineNum)
	{
		Log(LogLevel::Warn, tag, msg, file, funcName, lineNum);
	}

	inline void Error(const std::string& tag, const std::string& msg,
		const char* file, const char* funcName, unsigned int lineNum)
	{
		//Add message window which will allow to ignore error

		Log(LogLevel::Error, tag, msg, file, funcName, lineNum);
	}

	inline void FatalError(const std::string& tag, const std::string& msg,
		const char* file, const char* funcName, unsigned int lineNum)
	{
		Log(LogLevel::FatalError, tag, msg, file, funcName, lineNum);

		std::string windowMsg = std::format("{}\n{}\n{} {}", msg, file, funcName, lineNum);

		// Message window which automatically ends program, no option of ignoring
		SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"FatalError",
			windowMsg.c_str(),
			NULL
		);
	}
#pragma endregion Logging
}