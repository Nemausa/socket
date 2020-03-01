#ifndef CELL_TASK_HPP_
#define CELL_TASK_HPP_

/**
* @file cell_task.hpp
*
* @brief class for handling network messages
*
* @date	2020-03-01
* @author morris
* contact: tappanmorris@outlook.com
*
*/

#include <thread>
#include <mutex>
#include <list>

class CellTask
{
public:
	CellTask()
	{

	}
	virtual ~CellTask()
	{

	}

	// 执行任务
	virtual void work()
	{

	}

private:

};

class CellTaskServer:public CellTask
{
public:
	void add_task(CellTask* task)
	{
		std::lock_guard<std::mutex> lg(mutex_);
		task_buf_.push_back(task);
		
	}

	void start()
	{
		// thread
		std::thread t(std::mem_fn(&CellTaskServer::on_run), this);
		t.detach();
	}

	void on_run()
	{
		while (true)
		{
			// get data from task buffer
			if (!task_buf_.empty())
			{
				std::lock_guard<std::mutex> lg(mutex_);

				for (auto task : task_buf_)
				{
					task_list_.push_back(task);
				}
				task_buf_.clear();
			}
			if(task_list_.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			// do task
			for (auto task:task_list_)
			{
				task->work();
				delete task;
			}

			task_list_.clear();

		}
		
		
	}

private:
	std::list<CellTask*> task_list_;  // task data
	std::list<CellTask*> task_buf_;   // task data buffer
	std::mutex mutex_;  // need to lock when changing the data buffer
};

#endif//CELL_TASK_HPP_