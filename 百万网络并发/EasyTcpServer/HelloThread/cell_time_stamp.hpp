#ifndef CELL_TIME_STAMP_HPP_
#define CELL_TIME_STAMP_HPP_

/**
* @file cell_time_stamp.hpp
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-02-24
* @author Morris
* contact: tappanmorris@outlook.com
*
*/
#include <chrono>
using namespace std::chrono;

class CellTimeStamp
{
public:
	CellTimeStamp()
	{
		update();
	}
	virtual ~CellTimeStamp()
	{

	}

	void update()
	{
		begin_ = high_resolution_clock::now();
	}

	double get_elapsed_second()
	{
		return get_elapsed_microseconds()*0.000001;
	}

	double get_elapsed_millisecond()
	{
		return get_elapsed_microseconds()*0.001;
	}

	long long get_elapsed_microseconds()
	{
		return duration_cast<microseconds>(high_resolution_clock::now()-begin_).count();
	}

private:

protected:
	time_point<high_resolution_clock> begin_;
};


#endif