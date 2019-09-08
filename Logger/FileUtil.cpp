

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "FileUtil.h"

AppendFile::AppendFile(std::string filename)
	:fp_(fopen(filename.c_str(), "ae"))
{
	//打开文件之后，使用用户提供的缓冲区
	setbuffer(fp_, buffer_, sizeof(buffer_));
}

//写到文件
size_t AppendFile::write(const char* logline, size_t len)
{
	//不可重入版本的fwrite
	return fwrite_unlocked(logline, 1, len, fp_);
}

void AppendFile::flush()
{
	fflush(fp_);
}


void AppendFile::append(const char* logline, const size_t len)
{
	size_t n = this->write(logline, len);
	size_t remain = len - n;

	//如果一次未写完，保证数据持续写入
	while (remain > 0)
	{
		size_t x = this->write(logline + n, remain);
		if (x == 0)//写入完成或者失败
		{
			int err = ferror(fp_);
			if (err)
				fprintf(stderr, "AppendFile::append() failed !\n");
			break;
		}
		n += x;
		remain = len - n;
	}
}

AppendFile::~AppendFile()
{
	fclose(fp_);
}