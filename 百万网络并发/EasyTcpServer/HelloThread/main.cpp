#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include "cell_time_stamp.hpp"
using namespace std;

mutex m;
const int thread_size=4;
atomic_int sum = 0;

void work_function(int index)
{
	
	for (int n = 0; n < 2000000; n++)
	{
		//lock_guard<mutex> lg(m);
		 // 临界区域 开始
		sum++;
		// 临界区域 结束
	
	}
	// 线程安全  线程不安全
	
}

int main()
{
	thread t[thread_size];
	for (int n = 0; n < thread_size; n++)
	{
		t[n] = thread(work_function, n);
	}

	CellTimeStamp timer;
	for (int n = 0; n < thread_size; n++)
	{
		t[n].join();
	}
	cout << timer.get_elapsed_millisecond() << ",sum" << sum << endl;
	

	for (int n = 0; n < 4; n++)
		cout << "hello, main thread" << endl;
	
	return 0;
}