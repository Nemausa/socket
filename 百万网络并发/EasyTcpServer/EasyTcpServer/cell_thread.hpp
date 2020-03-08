#ifndef CELL_THREAD_HPP_
#define CELL_THREAD_HPP_

/**
* @file cell_thread.hpp
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-03-08
* @author morris
* contact: tappanmorris@outlook.com
*
*/
#include "cell_signal.hpp"

class CellThread
{
private:
	typedef std::function<void(CellThread*)> EventCall;

public:
	CellThread()
	{
		is_start_ = false;
	}
	~CellThread() 
	{
		
	}

	void start(
		EventCall on_create = nullptr,
		EventCall on_run = nullptr, 
		EventCall on_destory = nullptr)
	{
		std::lock_guard<std::mutex> lg(mutex_);
		if (is_start_)
			return;

		is_start_ = true;
		if (on_create)
			on_create_ = on_create;
		if (on_run)
			on_run_ = on_run;
		if (on_destory)
			on_destory_ = on_destory;

		
		std::thread t(std::mem_fn(&CellThread::on_work), this);
		t.detach();

		

	}

	void close()
	{
		std::lock_guard<std::mutex> lg(mutex_);
		if (!is_start_)
			return;
		is_start_ = false;
		signal_.wait();

	}

	// 在工作函数中退出，不需要使用信号量
	void exit()
	{
		std::lock_guard<std::mutex> lg(mutex_);
		if (is_start_)
			is_start_ = false;
	}

	bool is_run()
	{
		return is_start_;
	}


protected:
	// 线程运行时的工作函数
	void on_work()
	{
		if (on_create_)
			on_create_(this);
		if (on_run_)
			on_run_(this);
		if (on_destory_)
			on_destory_(this);

		signal_.wakeup();

	}

private:
	EventCall on_create_;
	EventCall on_run_;
	EventCall on_destory_;
	CellSignal signal_;	// 控制线程的终止
	std::mutex mutex_;	// 改变数据是需要加锁
	bool is_start_;		// 是否启动运行中
};



#endif  // !CELL_THREAD_HPP_