#include <stdlib.h>
#include <mutex>
#include <iostream>
#include "memory_mg.hpp"
#include "cell_time_stamp.hpp"



const int t_count = 8;
const int m_count = 100000;
const int n_count = m_count / t_count;

void wrokFun(int id)
{
	char* data[n_count];
	for (size_t i = 0; i < n_count; i++)
	{
		data[i] = new char[rand()%128 + 1];
	}

	for (size_t i = 0; i < n_count; i++)
	{
		delete[] data[i];
	}
}

int main()
{

	double sum = 0;
	std::thread t[t_count];
	for (int n = 0; n < t_count;n++)
	{
		t[n] = std::thread(wrokFun, n);
	}
	CellTimeStamp timer;
	for (int n = 0; n < t_count; n++)
	{
		t[n].join();
	}

	std::cout << timer.get_elapsed_millisecond() << ",sum= " << sum << std::endl;
	
	timer.update();

	

	return 0;
}