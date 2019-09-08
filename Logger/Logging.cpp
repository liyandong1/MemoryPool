
#include <thread>
#include <assert.h>
#include <time.h>
#include <mutex>
#include <chrono>
#include <string>
#include <sstream>
#include <map>
#include "Logging.h"
#include "AsyncLogging.h"


#include <iostream>   
using namespace std;

static std::map<Level, const char*> LogLevelMap
{
	{Level::Debug, "Debug"},
	{Level::Info, "Info"},
	{Level::Warning, "Warning"},
	{Level::Error, "Error"},
	{Level::Fatal, "Fatal"}
};

//在多线程程序执行时，某些事情只需要做一次
//call_once能保证函数纸杯调用一次

static std::once_flag onceFlag;//系统的一个状态位，可以确保函数是否被调用过
static AsyncLogging* AsyncLogger_;//它只能被创建一次
std::string Logger::logFileName_ = "Server.log";

//初始化AsyncLogging对象，确保只有这一个组件
//这个函数只能被调用一次

void once_init()
{
	AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
	AsyncLogger_->start();
}

//转而调用AsyncLogger_异步日志组件，完成写操作
void output(const char* msg, int len)
{
	std::call_once(onceFlag, once_init);
	//可以用Async组件写
	AsyncLogger_->append(msg, len);
}


Logger::Impl::Impl(const char* fileName, int line, Level level)
	:basename_(fileName),line_(line),level_(level),stream_()
{
	formatTime();
}

//采用C++11
void Logger::Impl::formatTime()
{
	//先在当前缓冲区的头部添加时间信息
	auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	struct tm* p_time = localtime(&currentTime);
	char str_t[26] = { 0 };
	strftime(str_t, 26, "%Y-%m-%d %H:%M:%S   ", p_time);
	stream_ << str_t;
	//再添加等级信息
	stream_ << "[Level::" << LogLevelMap[level_] << "]   ";
}

Logger::Logger(const char* fileName, int line, Level level)
	:impl_(fileName, line, level)
{

}

LogStream& Logger::stream()
{
	return impl_.stream_;
}

Logger::~Logger()
{
	//得到线程id
	std::ostringstream oss;
	oss << std::this_thread::get_id();
	
	impl_.stream_ << "   -- " << impl_.basename_ << ':' << impl_.line_ << 
					" ThreadId:" << oss.str() << '\n';
	const LogStream::Buffer& buf(stream().buffer());
	output(buf.data(), buf.length());
}

void Logger::setLogFileName(std::string fileName)
{
	logFileName_ = fileName;
}

std::string Logger::getLogFileName()
{
	return logFileName_;
}
