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
#include <functional>
#include "cell_thread.hpp"

class CellTaskServer
{
	typedef std::function<void()> CellTask;
public:
	void add_task(CellTask task)
	{
		std::lock_guard<std::mutex> lg(mutex_);
		task_buf_.push_back(task);
		
	}

	void start()
	{
		thread_.start(nullptr, [this](CellThread* pthread) {
			on_run(pthread);
		}, nullptr);
	}

	void exit()
	{
		printf("CellTaskServer%d.close begin\n", server_id_);
		thread_.close();
		printf("CellTaskServer%d.close end\n", server_id_);
		
	}


	void on_run(CellThread* pthread)
	{
		while (pthread->is_run())
		{
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

			for (auto task:task_list_)
				task();

			task_list_.clear();

		}	
		printf("CellTaskServer%d.on_run exit\n", server_id_);
		
	}
public:
	int server_id_=-1;  // 所属server的id
private:
	std::list<CellTask> task_list_;		// task data
	std::list<CellTask> task_buf_;		// task data buffer
	std::mutex mutex_;					// need to lock when changing the data buffer
	CellThread thread_;

};

#endif  //CELL_TASK_HPP_