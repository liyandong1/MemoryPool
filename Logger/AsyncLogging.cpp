
#include <chrono>
#include "LogFile.h"
#include "AsyncLogging.h"

#include <iostream>
using namespace std;

AsyncLogging::AsyncLogging(const std::string basename, int flushInterval)
	:basename_(basename), flushInterval_(flushInterval), running_(false),
	thread_(std::bind(&AsyncLogging::threadFunc, this)), mutex_(),
	currentBuffer_(new Buffer),nextBuffer_(new Buffer),buffers_(),latch_(1)
{
	assert(basename_.size() > 1);
	currentBuffer_->bzero();
	nextBuffer_->bzero();
	buffers_.reserve(16);//预留空间
}

void AsyncLogging::threadFunc()
{
	{
		//等待start函数执行，线程函数才执行
		std::unique_lock<std::mutex> lk(mutex_);
		cond_.wait(lk, [this]{return running_;});  //等待satrt了才真正执行
		latch_.countDown();//线程函数真正启动了，外层start可以返回
	}
	
	//用类管理日志输出文件
	LogFile output(basename_);//通过它写日志
	
	//额外的两块缓冲，实际上是四缓冲
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero();
	newBuffer1->bzero();
	
	//需要写入磁盘的所有缓冲都放那个里面
	BufferPtrVector buffersToWrite;
	buffersToWrite.reserve(16);//预留空间
	
	//线程循环，执行写缓冲操作
	while (running_)
	{
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		assert(buffersToWrite.empty());

		//缓冲区需要临界
		{
			std::unique_lock<std::mutex> lk(mutex_);
			if (buffers_.empty())//没有需要写的
			{
				//至少每隔flushInterval_ s写一次磁盘，空就睡眠
				cond_.wait_for(lk, std::chrono::seconds(flushInterval_));
			}
			buffers_.push_back(currentBuffer_);
			currentBuffer_.reset();

			currentBuffer_ = std::move(newBuffer1);

			//为了后面的代码能够安全的访问buffersToWrite
			buffersToWrite.swap(buffers_);

			//减少前端临界区的长度，减少前端分配缓冲的次数
			if (!nextBuffer_)
			{
				nextBuffer_ = std::move(newBuffer2);
			}

		}

			assert(!buffersToWrite.empty());
			
			//
			if (buffersToWrite.size() > 25)
				buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
			
			//写入到文件（其实是缓冲）
			for (size_t i = 0; i < buffersToWrite.size(); ++i)
			{
				output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
			}

			if (buffersToWrite.size() > 2)
				buffersToWrite.resize(2);

			//把buffersToWrite里面的缓冲替换回newBuffer
			//冷热缓冲机制
			if (!newBuffer1)
			{
				assert(!buffersToWrite.empty());
				newBuffer1 = buffersToWrite.back();
				buffersToWrite.pop_back();
				newBuffer1->reset();
			}

			if (!newBuffer2)
			{
				assert(!buffersToWrite.empty());
				newBuffer2 = buffersToWrite.back();
				buffersToWrite.pop_back();
				newBuffer2->reset();
			}

			buffersToWrite.clear();
			output.flush();//写到磁盘
		}
	output.flush();
}

void AsyncLogging::append(const char* logline, int len)
{
	std::unique_lock<std::mutex> lk(mutex_);
	if (currentBuffer_->avail() > len)
	{//当前缓冲能放下
		currentBuffer_->append(logline, len);
	}
	else//换一块缓冲
	{
		buffers_.push_back(currentBuffer_);//存放数据
		currentBuffer_.reset();

		//一定要找到下一块可用缓冲
		if (nextBuffer_)
			currentBuffer_ = std::move(nextBuffer_);
		else
			currentBuffer_.reset(new Buffer);
		currentBuffer_->append(logline, len);
		cond_.notify_all();   //通知io线程可以进行写磁盘的io操作了
	}
	
}

//要注意thread创建起来就启动了
void AsyncLogging::start()
{
	running_ = true;
	cond_.notify_all();// 让休眠线程唤醒
	latch_.wait();//为了确保threadFunc真正的执行了，当前satrt函数才返回
}

void AsyncLogging::stop()
{
	running_ = false;
	cond_.notify_all(); //唤醒所有的线程
	thread_.join(); //回收线程
}

AsyncLogging::~AsyncLogging()
{
	if (running_)
		stop();
}
