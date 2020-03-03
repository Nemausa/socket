#ifndef MEMORY_MG_
#define MEMORY_MG_

/**
* @file memory_mg.h
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-03-02
* @author morris
* contact: tappanmorris@outlook.com
*
*/
#include <stdlib.h>
#include <assert.h>
#include <mutex>


#ifdef _DEBUG
#include <stdio.h>
	#define xprintf(...) printf(__VA_ARGS__)
#else
	#define xprintf(...)
#endif

#define MAX_MEMORY_SIZE 1024

class MemoryAlloc;
// 內存块
class MemoryBlock
{
public:
	MemoryBlock() {}
	~MemoryBlock() {}

public:
	int id_;	// 内存块编号
	int ref_;	// 引用次数
	MemoryAlloc* alloc_;  // 所属内存池
	MemoryBlock* next_;   // 下一块位置
	bool in_pool_;	// 是否在内存池中
private:
	// 预留
	char c1;
	char c2;
	char c3;
};

//const int  MemoryBlockSize = sizeof(MemoryBlock);
// 內存池
class MemoryAlloc
{
public:
	MemoryAlloc() 
	{
		address_ = nullptr;
		header_ = nullptr;
		block_size_ = 0;
		block_num_ = 0;
		xprintf("MemoryAlloc\n");
	}
	~MemoryAlloc() 
	{
		if (address_)
			free(address_);
	}
	// 初始化内存池
	void init_memory()
	{
		xprintf("MemoryAlloc:size=%d, block_num=%d \n", block_size_, block_num_);
		assert(nullptr==address_);
		if (address_)
			return;
		int info_size = sizeof(MemoryBlock);
		// 向系统申请池内存
		size_t real_size = block_size_ + info_size;
		size_t sum_size = block_num_ * real_size;
		address_ = (char*)malloc(sum_size);

		// 初始化内存池
		header_ = (MemoryBlock*)address_;
		header_->id_ = 0;
		header_->ref_ = 0;
		header_->in_pool_ = true;
		header_->alloc_ = this;
		header_->next_ = nullptr;

		
		MemoryBlock* pre = header_;
		// 遍历内存块进行初始化
		for (size_t n = 1; n < block_num_;n++)
		{
			MemoryBlock* temp = (MemoryBlock*)(address_ + n*real_size);
			temp->id_ = n;
			temp->ref_ = 0;
			temp->in_pool_ = true;
			temp->alloc_ = this;
			temp->next_ = nullptr;
			pre->next_ = temp;
			pre = temp;
		}

	}

	void* alloc_mem(size_t size)
	{
		std::lock_guard<std::mutex> lg(mutex_);
		if (!address_)
			init_memory();

		MemoryBlock* block = nullptr;
		if (nullptr == header_)  // 内存池无可用块
		{
			block = (MemoryBlock*)malloc(size + sizeof(MemoryBlock));
			block->id_ = -1;
			block->ref_ = 1;
			block->in_pool_ = false;
			block->alloc_ = this;
			block->next_ = nullptr;
		}
		else
		{
			block = header_;
			header_ = header_->next_;
			assert(0 == block->ref_);
			block->ref_ = 1;
		}
		xprintf("allocmem:%10llx, id=%5d, size=%5d \n", block, block->id_, size);
		return ((char*)block+sizeof(MemoryBlock));

	}

	void free_memory(void* mem)
	{
		MemoryBlock* block = (MemoryBlock*)((char*)mem - sizeof(MemoryBlock));
		assert(1 == block->ref_);
		
		if (block->in_pool_)
		{
			std::lock_guard<std::mutex> lg(mutex_);
			if (--block->ref_ != 0)
			{
				return;
			}
			block->next_ = header_;
			header_ = block;
		}
		else
		{
			if (--block->ref_ != 0)
			{
				return;
			}
			free(block);
		}

	}
protected:
	char* address_;			// 内存池地址
	MemoryBlock* header_;	// 内存头部单元
	size_t block_size_;		// 内存单元大小
	size_t block_num_;		// 内存单元的数量
	std::mutex mutex_;
};

// 便于在声明成员变量的嘶吼后初始化MemoryAlloc的数据成员
template<size_t block_num, size_t block_size>
class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		const size_t n = sizeof(void*);

		block_size_ = (block_size / n)*n + (block_size % n ? n : 0);

		block_num_ = block_num;
		block_size_ = block_size;
	}
};



// 內存管理
class MemoryMg
{
private:
	MemoryMg()
	{
		init_map(0, 64, &mem64_);
		init_map(65, 128, &mem128_);
		init_map(129, 256, &mem256_);
		init_map(257, 512, &mem512_);
		init_map(513, 1024, &mem1024_);
	}
	~MemoryMg()
	{

	}

public:
	static MemoryMg& Instance()
	{
		static MemoryMg mg;
		return mg;
	}

	// 申请内存
	void* alloc_mem(size_t size)
	{
		if (size <= MAX_MEMORY_SIZE)
		{
			return sz_alloc_[size]->alloc_mem(size);
		}
		else
		{
			// 向系统申请内存
			MemoryBlock* block = (MemoryBlock*)malloc(size + sizeof(MemoryBlock));
			block->id_ = -1;
			block->ref_ = 1;
			block->in_pool_ = false;
			block->alloc_ = nullptr;
			block->next_ = nullptr;
			xprintf("allocmem:%10llx, id=%5d, size=%5d \n", block, block->id_, size);
			return (char*)block+sizeof(MemoryBlock);
		}

	}

	// 释放内存
	void free_mem(void* mem)
	{
		MemoryBlock* block = (MemoryBlock*)((char*)mem - sizeof(MemoryBlock));
		xprintf("freemem: %10llx, id=%5d \n", block, block->id_);
		if (block->in_pool_)
		{
			block->alloc_->free_memory(mem);
		}
		else
		{
			if(--block->ref_==0)
				free(block);
		}

	}

	void add_ref(void* mem)
	{
		MemoryBlock* block = (MemoryBlock*)((char*)mem - sizeof(MemoryBlock));
		block->ref_++;
	}
private:
	// 初始化内存池映射数组
	void init_map(int begin, int end, MemoryAlloc* mem)
	{
		for (int n = begin; n <= end; n++)
		{
			sz_alloc_[n] = mem;
		}
	}
private:
	MemoryAlloctor<100000, 64>	mem64_;
	MemoryAlloctor<100000, 128> mem128_;
	MemoryAlloctor<100000, 256> mem256_;
	MemoryAlloctor<100000, 512> mem512_;
	MemoryAlloctor<100000, 1024> mem1024_;
	MemoryAlloc* sz_alloc_[MAX_MEMORY_SIZE + 1];


};





#endif  // MEMORY_MG_