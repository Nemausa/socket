#ifndef CELL_SIGNAL_HPP_
#define CELL_SIGNAL_HPP_

/**
* @file cell_signal.hpp
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

#include <chrono>
#include <thread>
#include <condition_variable>

class CellSignal
{
public:
	void wait()
	{
		std::unique_lock<std::mutex> ul(mutex_);
		if(--wait_<0)
		{
			cv_.wait(ul, [this]()->bool {
				return wakeup_ > 0;
			});
			--wakeup_;
		}
			
	}

	void wakeup()
	{
		std::lock_guard<std::mutex> lg(mutex_);
		if (++wait_<=0)
		{
			++wakeup_;
			cv_.notify_one();
		}

	}
private:
	int wait_ = 0;		// 等待计数
	int wakeup_ = 0;	// 唤醒次数
	std::mutex mutex_;
	std::condition_variable cv_;
};

#endif  // !CELL_SIGNAL_HPP_