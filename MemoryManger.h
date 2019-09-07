#ifndef _MAMORYMANGER_H_
#define _MEMORYmANGER_H_

#define MAX_MEMORY_SIZE 1024

#include <mutex>

class MemoryPool;

//内存块头部信息
struct MemoryBlockHead
{
public:
	MemoryPool* ownerMemoryPool_;//所属内存池
	MemoryBlockHead* pNext_;//下一个磁盘块
	int blockId_;
	int blockRef_;
	bool isInMemoryPool_;//是否属于内存池中内存
private:
	char c1;
	char c2;
	char c3;
};

//const int a = sizeof(MemoryBlockHead);

class MemoryPool
{
protected:
	size_t currentMemBlockSize_;  //每一块的大小
	size_t blockCount_;

	char* pBuf_;//系统申请的内存首地址
	MemoryBlockHead* pHead_; //链栈管理内存块，指向当前可以申请的内存块

	std::mutex m_mutex; //多线程

	//初始化内存池
	void initMemoryPool();
public:
	void* alloMemory(size_t size);
	void deAllocMemory(void* p);
	MemoryPool();
	~MemoryPool();
};

template <size_t memBlockSize, size_t count>
class SpecificMemoryPool:public MemoryPool
{
public:
	SpecificMemoryPool()
	{
		const size_t n = sizeof(void*);
		currentMemBlockSize_ = ((memBlockSize / n) * n) + ((memBlockSize% n == 0) ? 0 : n);
		blockCount_ = count;
	}

	~SpecificMemoryPool() {}
};

class MemoryManger
{
private:
	SpecificMemoryPool<64, 10> mem64_;
	SpecificMemoryPool<128, 10> mem128_;
	SpecificMemoryPool<256, 10> mem256_;
	SpecificMemoryPool<512, 10> mem512_;
	SpecificMemoryPool<1024, 10> mem1024_;

	MemoryPool* memoryPoolMap[MAX_MEMORY_SIZE + 1];//内存池映射数组

	MemoryManger(const MemoryManger&) = delete;
	MemoryManger& operator=(const MemoryManger&) = delete;
	MemoryManger();

	//为区域范围的内存块建立映射关系
	void createMapRelatition(size_t begin, size_t end, MemoryPool* pMem);
public:
	//静态单例类的写法
	static MemoryManger& getInstance();
	//分配内存
	void* allocMemory(size_t size);
	void deAllocMemory(void* p);
	~MemoryManger();
};



#endif
