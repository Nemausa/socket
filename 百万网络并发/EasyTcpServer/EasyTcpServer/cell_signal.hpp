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

class CellSignal
{
public:
	CellSignal() 
	{
		is_wait_ = false;
	}
	~CellSignal() 
	{
	
	}

	void wait()
	{
		is_wait_ = true;
		while (is_wait_)
		{
			std::chrono::milliseconds dura(1);
			std::this_thread::sleep_for(dura);
		}
	}

	void wakeup()
	{
		if (is_wait_)
			is_wait_ = false;
		else
			printf("CELLSemaphore wakeup error.");
	}
private:
	bool is_wait_;
};

#endif  // !CELL_SIGNAL_HPP_