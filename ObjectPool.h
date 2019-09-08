#ifndef _OBJECTPOOL_H_
#define _OBJECTPOOL_H_

#include <mutex>
#include <stdlib.h>
#include <assert.h>

#include "MemoryControl.h"

template <typename T, size_t objectPoolSize>
class ObjectPool
{
private:
	//对象头部结构
	struct ObjectNodeHeader
	{
	public:
		
		ObjectNodeHeader* pNext_; //下一个节点位置
		size_t objectId_;     
		char objectRef_;    //引用次数
		bool isInObjectPool_;
	private:
		char c1;
		char c2;
	};

	ObjectNodeHeader* pHead_;
	char* pBuf_;  //对象池首地址
	std::mutex m_mutex;

	//初始化对象池
	void initObjectPool()
	{
		assert(pBuf_ == nullptr);
		size_t allSize = (sizeof(T) + sizeof(ObjectNodeHeader)) * objectPoolSize;
		//这里使用new，优先向内存池申请内存
		pBuf_ = new char[allSize];

		//用链表串起来
		pHead_ = reinterpret_cast<ObjectNodeHeader*>(pBuf_);
		pHead_->pNext_ = nullptr;
		pHead_->objectId_ = 0;
		pHead_->objectRef_ = 1;
		pHead_->isInObjectPool_ = true;

		ObjectNodeHeader* pTail = pHead_;
		for (int i = 1; i < objectPoolSize; i++)
		{
			ObjectNodeHeader* pCurrent = reinterpret_cast<ObjectNodeHeader*>(pBuf_ + i*(sizeof(ObjectNodeHeader) + sizeof(T)));
			pCurrent->pNext_ = nullptr;
			pCurrent->objectId_ = 1;
			pCurrent->objectRef_ = 1;
			pCurrent->isInObjectPool_ = true;

			pTail->pNext_ = pCurrent;
			pTail = pTail->pNext_;
		}
	}
public:
	//const int n = sizeof(ObjectNodeHeader

	ObjectPool()
	{
		pBuf_ = nullptr;
		pHead_ = nullptr;
		initObjectPool();
	}
	
	//分配对象内存
	void* allocMemory(size_t size)
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		assert(pBuf_ != nullptr);
			
		if (pHead_ != nullptr)//从对象池中找对象
		{
			ObjectNodeHeader* pCurrent = pHead_;
			pHead_ = pHead_->pNext_;
			return reinterpret_cast<char*>(pCurrent) + sizeof(ObjectNodeHeader);
		}
		else
		{
			//优先向内存池申请内存
			char* p = new char[sizeof(T) + sizeof(ObjectNodeHeader)];
			ObjectNodeHeader* pCurrent = reinterpret_cast<ObjectNodeHeader*>(p);
			pCurrent->pNext_ = nullptr;
			pCurrent->objectId_ = 0;
			pCurrent->objectRef_ = 1;
			pCurrent->isInObjectPool_ = true;
			return reinterpret_cast<char*>(pCurrent) + sizeof(ObjectNodeHeader);
		}
	}
	//回收对象内存,一样要注意偏移问题
	void deAllocMemory(void* p)
	{
		char* q = reinterpret_cast<char*>(p) - sizeof(ObjectNodeHeader);
		ObjectNodeHeader* pCurrent = reinterpret_cast<ObjectNodeHeader*>(q);
		assert(pCurrent->objectRef_ == 1);
		pCurrent->objectRef_ = 0;
		if (pCurrent->isInObjectPool_ == true)
		{
			std::lock_guard<std::mutex> lk(m_mutex);
			pCurrent->pNext_ = pHead_;//头插法
			pHead_ = pCurrent; 
		}
		else
		{
			delete[] q;
		}
	}

	~ObjectPool()
	{
		if(pBuf_ != nullptr)
			delete[] pBuf_;
	}
};



//提供一组基类的接口，让需要使用对象池的对象继承此基类
template <typename T, size_t objectPoolSize>
class ObjectPoolBase
{
private:
	static ObjectPool<T, objectPoolSize>& getObjectPool()
	{
		//静态对象池对象
		static ObjectPool<T, objectPoolSize> objPool_;
		return objPool_;
	}
public:
	void* operator new(size_t size)
	{
		return getObjectPool().allocMemory(size);
	}

	void operator delete(void* p)
	{
		getObjectPool().deAllocMemory(p);
	}

	//提供这两个方法而不是直接暴露接口，可以做一些想做的事

	//如果类的构造函数有多个参数，统一化使用变参模板
	template <typename ...Args>
	static T* createObject(Args... args)
	{
		return new ObjectPoolBase(args...);
	}

	static void destoryObject(T* obj)
	{
		delete obj;
	}
};

#endif