#ifndef CELL_OBJECT_POOL_HPP_
#define CELL_OBJECT_POOL_HPP_

/**
* @file cell_object_pool.hpp
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-03-04
* @author morris
* contact: tappanmorris@outlook.com
*
*/

#include <stdlib.h>
#include <mutex>
#include <assert.h>

#ifdef _DEBUG
#include <stdio.h>
	#ifndef xprintf
	#define xprintf(...) printf(__VA_ARGS__)
	#endif
#else
	#ifndef xprintf
	#define xprintf(...) 
	#endif	
#endif

template<class Type, size_t pool_num>
class CellObjectPool
{
public:
	CellObjectPool() 
	{
		init_pool();
	}
	virtual ~CellObjectPool() 
	{
		if(address_)
			delete[] address_;
	}
public:
	// 释放对象
	void free_memory(void* p)
	{
		NodeHeader* pool = (NodeHeader*)((char*)p - sizeof(NodeHeader));
		assert(1 == pool->ref_);
		if (pool->in_pool_)
		{
			std::lock_guard<std::mutex>lg(mutex_);
			if (--pool->ref_ != 0)
			{
				return;
			}
			pool->next_ = header_;
			header_ = pool;
			xprintf("freeObjmem:%p, id=%d \n", pool, pool->id_);
		}
		else
		{
			
			if (--pool->ref_ != 0)
			{
				return;
			}
			delete[] pool;
			xprintf("freeObjmem:%p, id=%d \n", pool, pool->id_);

		}

	}
	// 申请对象
	void* alloc_memory(size_t size)
	{
		std::lock_guard<std::mutex> lg(mutex_);
		NodeHeader* pool = nullptr;
		if (nullptr == header_)  // 内存池无可用块
		{
			pool = (NodeHeader*)new char[sizeof(Type) + sizeof(NodeHeader)];
			pool->id_ = -1;
			pool->ref_ = 1;
			pool->in_pool_ = false;
			pool->next_ = nullptr;
		}
		else
		{
			pool = header_;
			header_ = header_->next_;
			assert(0 == pool->ref_);
			pool->ref_ = 1;
		}
		xprintf("allocObjmem:%p, id=%5d, size=%5d \n", pool, pool->id_, size);
		return ((char*)pool + sizeof(NodeHeader));
	}

	// 初始化对象池
	void init_pool() 
	{
		if (address_)
			return;
		size_t real_size = sizeof(Type) + sizeof(NodeHeader);
		size_t n = pool_num*real_size;
		address_ = new char[n];

		// 初始化内存池
		header_ = (NodeHeader*)address_;
		header_->id_ = 0;
		header_->ref_ = 0;
		header_->in_pool_ = true;
		header_->next_ = nullptr;

		NodeHeader* pre = header_;
		// 遍历内存块进行初始化
		for (size_t n = 1; n < pool_num; n++)
		{
			NodeHeader* temp = (NodeHeader*)(address_ + n*real_size);
			temp->id_ = n;
			temp->ref_ = 0;
			temp->in_pool_ = true;
			temp->next_ = nullptr;
			pre->next_ = temp;
			pre = temp;
		}
	}

private:
	struct NodeHeader
	{
		int  id_;	// 内存块编号
		char ref_;	// 引用次数
		bool in_pool_;	// 是否在内存池中
		NodeHeader* next_;   // 下一块位置
		// 预留
		char c1;
		char c2;
	};

private:
	char* address_;			// 对象池地址
	NodeHeader* header_;	
	std::mutex mutex_;
	
};

template<class Type, size_t pool_num>
class ObjectPoolBase
{
public:
	ObjectPoolBase() {}
	~ObjectPoolBase() {}

	void* operator new(size_t size)
	{
		return  object_pool().alloc_memory(size);
	}
	void operator delete(void* p)
	{
		object_pool().free_memory(p);
	}

	// 不定参数的构造函数
	template<typename ...Args>
	static Type* create_objetc(Args... args)
	{
		Type* obj = new Type(args...);
		return obj;
	}

	static void destroy_object(Type* obj)
	{
		delete obj;
	}

private:
	typedef CellObjectPool<Type, pool_num> ClaasTypePool;
	static ClaasTypePool& object_pool()
	{
		static ClaasTypePool pool;
		return pool;
	}
};




#endif  // CELL_OBJECT_POOL_HPP_