
#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <string.h>
#include <string>
#include <stdio.h>
#include "LogStream.h"

enum class Level { Debug = 0, Info, Warning, Error, Fatal };//日志级别

class Logger
{
private:
	class Impl
	{
	public:
		std::string basename_;
		int line_;
		Level level_;//转换为字符串
		LogStream stream_;

		Impl(const char* fileName, int line, Level level);
		//先在当前缓冲区的头部添加时间信息,级别信息
		void formatTime();
	};

	Impl impl_;//可以对外提供一块格式化完成后的缓冲
	static std::string logFileName_;
public:
	Logger(const char* fileName, int line, Level level);
	LogStream& stream();
	~Logger();

	static void setLogFileName(std::string fileName);
	static std::string getLogFileName();
};

#define LOG_DEBUG   Logger(__FILE__, __LINE__, Level::Debug).stream()
#define LOG         Logger(__FILE__, __LINE__, Level::Info).stream()
#define LOG_WARNING Logger(__FILE__, __LINE__, Level::Warning).stream()
#define LOG_ERROR   Logger(__FILE__, __LINE__, Level::Error).stream()
#define LOG_FATAL   Logger(__FILE__, __LINE__, Level::Fatal).stream()

#endif
