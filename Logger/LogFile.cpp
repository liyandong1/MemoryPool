
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "LogFile.h"

LogFile::LogFile(const std::string& basename, int flushEveryN)
	:basename_(basename),flushEveryN_(flushEveryN),
	 count_(0), mutex_(new std::mutex)
{
	file_.reset(new AppendFile(basename_));
}

//flushEveryN_次就刷一下缓冲
void LogFile::append_unlocked(const char* logline, int len)
{
	file_->append(logline, len);
	++count_;
	if (count_ >= flushEveryN_)
	{
		file_->flush();
		count_ = 0;
	}
}

//要考虑多线程情况
void LogFile::append(const char* logline, int len)
{
	std::lock_guard<std::mutex> lk(*mutex_);
	append_unlocked(logline, len);
}

//其实不是flush到磁盘，而是flush到缓冲区
void LogFile::flush()
{
	std::lock_guard<std::mutex> lk(*mutex_);
	file_->flush();
}

//bool LogFile::rollFile()
//{

//}

LogFile::~LogFile()
{

}