
#ifndef _FILEUTIL_H_
#define _FILEUTIL_H_

#include <string>
#include "noncopyable.h"

const int MAX_BUFFER_SIZE = 64 * 1024;

//为文件封装一层
class AppendFile : public noncopyable
{
private:
	FILE* fp_;
	char buffer_[MAX_BUFFER_SIZE];//为写文件提供的应用层缓冲区
	//真实向文件写
	size_t write(const char* logline, size_t len);
public:
	explicit AppendFile(std::string filename);
	void flush();
	//append核心函数，提供给调用者接口，保证真实的把数据全部写入到磁盘
	void append(const char* logline, const size_t len);
	~AppendFile();
};

#endif