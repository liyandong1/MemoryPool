#ifndef _LOGSTREAM_H_
#define _LOGSTREAM_H_

#include <assert.h>
#include <string.h>
#include <string>
#include <thread>
#include "noncopyable.h"

#include <iostream>
using namespace std;

const int kSmallBuffer = 4096;
const int kLargeBuffer = 4096 * 1000;

//具体的缓冲
template <int SIZE>
class FixedBuffer : public noncopyable
{
private:
	char* cur_;
	char data_[SIZE];

	const char* end() const
	{ 
		return data_ + sizeof(data_); 
	}
public:
	FixedBuffer() :cur_(data_) {}

	//缓冲剩余空间
	int avail() const   
	{  
		return static_cast<int>(end() - cur_); 
	}

	//核心函数
	void append(const char* buf, size_t len)
	{
		//cout << "FixedBuffer::append " << buf << endl;
		//缓存可用空间
		if (avail() > static_cast<int>(len))
		{
			memcpy(cur_, buf, len);
			cur_ += len;
		}
	}

	const char* data() const 
	{ 
		return data_;
	}

	int length() const  
	{ 
		return static_cast<int>(cur_ - data_);
	}

	char* current()
	{
		return cur_;
	}

	void add(size_t len)
	{
		cur_ += len;
	}

	void reset()
	{
		cur_ = data_;
	}

	void bzero()
	{
		memset(data_, 0, sizeof(data_));
	}

	~FixedBuffer() {}
};


class LogStream : public noncopyable
{
public:
	using Buffer = FixedBuffer<kSmallBuffer>;
private:
	static const int kMaxNumericSize = 32;

	Buffer buffer_;

	//void staticCheck();

	template <typename T>
	void formatInteger(T);
public:
	LogStream() {}

	LogStream& operator<<(bool v);
	LogStream& operator<<(short);
	LogStream& operator<<(unsigned short);
	LogStream& operator<<(int);
	LogStream& operator<<(unsigned int);
	LogStream& operator<<(long);
	LogStream& operator<<(unsigned long);
	LogStream& operator<<(long long);
	LogStream& operator<<(unsigned long long);

	//LogStream& operator<<(const void*);
	LogStream& operator<<(double);
	LogStream& operator<<(long double);
	LogStream& operator<<(float v);
	LogStream& operator<<(char v);
	LogStream& operator<<(const char* str);
	LogStream& operator<<(const unsigned char* str);
	LogStream& operator<<(const std::string& v);

	//方便输出线程id到缓冲区
	LogStream& operator<<(const std::thread::id& tid);


	//每一个类都拥有一个append函数
	void append(const char* data, int len);
	const Buffer& buffer() const
	{
		return buffer_;
	}
	void resetBuffer();
	~LogStream();
};


#endif
