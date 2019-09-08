
/**
	启动一个log线程，用来专门将log写入logfile
	应用双缓冲技术，实际上有四块缓冲
*/

#ifndef _ASYNCLOGGING_H_
#define _ASYNCLOGGING_H_

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>

#include "noncopyable.h"
#include "LogStream.h"
#include "CountDownLatch.h"


//异步io核心
class AsyncLogging : public noncopyable
{
private:
	using Buffer = FixedBuffer<kLargeBuffer>; //封装的一块缓冲类型
	using BufferPtr = std::shared_ptr<Buffer>;
	using BufferPtrVector = std::vector<BufferPtr>;

	std::string basename_; //文件名
	const int flushInterval_;
	bool running_;

	std::thread thread_; //io线程
	std::mutex mutex_;
	std::condition_variable cond_;

	BufferPtr currentBuffer_;//当前正在使用的缓冲
	BufferPtr nextBuffer_; //下一块预备缓冲
	BufferPtrVector buffers_;
	CountDownLatch latch_;


	void threadFunc();// io线程执行函数,后端线程工作
public:
	AsyncLogging(const std::string basename, int flushInterval = 2);

	//只是把数据放到当前缓冲区里面,给前端线程调用
	void append(const char* logline, int len);
	void start(); //让线程开始io
	void stop();  //让线程停止io
	~AsyncLogging();
};

#endif