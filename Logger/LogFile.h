
/**
进一步封装AppendFile类，设置一个循环次数
每过那么多次就是flush一次
*/

#ifndef _LOGFILE_H_
#define _LOGFILE_H_

#include <mutex>
#include <memory>
#include <string>
#include "FileUtil.h"
#include "noncopyable.h"


class LogFile : public noncopyable
{
private:
	int count_;
	std::unique_ptr<std::mutex> mutex_;
	std::unique_ptr<AppendFile> file_;//真实的文件

	const int flushEveryN_;
	const std::string basename_;

	//不可重入
	void append_unlocked(const char* logline, int len);
public:
	LogFile(const std::string& basename, int flushEveryN = 1024);
	void append(const char* logline, int len);
	void flush();
	//bool rollFile();
	~LogFile();
};


#endif