
#include "LogStream.h"
#include <algorithm>
#include <limits>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <sstream>


#include <iostream>
using namespace std;

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

// Efficient Integer to String Conversions, by Matthew Wilson.
//高效的整数到字符串的转换
template<typename T>
size_t convert(char buf[], T value)
{
	T i = value;
	char* p = buf;

	do
	{
		int lsd = static_cast<int>(i % 10);
		i /= 10;
		*p++ = zero[lsd];
	} while (i != 0);

	if (value < 0)
	{
		*p++ = '-';
	}
	*p = '\0';
	std::reverse(buf, p);

	return p - buf;
}

//template class FixedBuffer<kSmallBuffer>;
//template class FixedBuffer<kLargeBuffer>;

//void LogStream::staticCheck()
//{

//}

template <typename T>
void LogStream::formatInteger(T v)
{
	//buffer容不下kMaxNumbericSize的话会直接被丢弃
	if (buffer_.avail() >= kMaxNumericSize)
	{
		size_t len = convert(buffer_.current(), v);
		buffer_.add(len);
	}
}

LogStream& LogStream::operator<<(bool v)
{
	buffer_.append((v ? "1" : "0"), 1);
	return *this;
}

LogStream& LogStream::operator<<(short v)
{
	*this << static_cast<int>(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
	*this << static_cast<unsigned int>(v);
	return *this;
}

LogStream& LogStream::operator<<(int v)
{
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(long v)
{
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(long long v)
{
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
	formatInteger(v);
	return *this;
}

//LogStream& LogStream::operator<<(const void*);
LogStream& LogStream::operator<<(double v)
{
	//超过的话直接被丢弃
	if (buffer_.avail() >= kMaxNumericSize)
	{
		int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
		buffer_.add(len);
	}
	return *this;
}

LogStream& LogStream::operator<<(long double v)
{
	if (buffer_.avail() >= kMaxNumericSize)
	{
		int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12Lg", v);
		buffer_.add(len);
	}
	return *this;
}

LogStream& LogStream::operator<<(float v)
{
	*this << static_cast<double>(v);
	return *this;
}

LogStream& LogStream::operator<<(char v)
{
	buffer_.append(&v, 1);
	return *this;
}

LogStream& LogStream::operator<<(const char* str)
{
	if (str != nullptr)
		buffer_.append(str, strlen(str));
	else
		buffer_.append("(null)", 6);
	return *this;
}

LogStream& LogStream::operator<<(const unsigned char* str)
{
	return operator<<(reinterpret_cast<const char*>(str));
}

LogStream& LogStream::operator<<(const std::string& v)
{
	buffer_.append(v.c_str(), v.size());
	return *this;
}

LogStream& LogStream::operator<<(const std::thread::id& tid)
{
	std::ostringstream oss;
	oss << tid;
	return *this << oss.str();
}

void LogStream::append(const char* data, int len)
{
	buffer_.append(data, len);
}


void LogStream::resetBuffer()
{
	buffer_.reset();//指针直接重新指向首地址就行了
}

LogStream::~LogStream()
{

}
