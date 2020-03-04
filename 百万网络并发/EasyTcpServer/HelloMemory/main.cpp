#include <stdlib.h>

#include <mutex>
//#include <memory>
#include <iostream>
//#include "memory_mg.hpp"
//#include "cell_time_stamp.hpp"
#include "cell_object_pool.hpp"
using namespace std;


const int t_count = 8;
const int m_count = 100000;
const int n_count = m_count / t_count;

class ClassA: public ObjectPoolBase<ClassA>
{
public:
	ClassA(int m)
	{
		printf("ClassA\n");
	}
	~ClassA()
	{
		printf("~ClassA\n");
	}

public:
	int num_;
};

class ClassB : public ObjectPoolBase<ClassB>
{
public:
	ClassB(int m, int n)
	{
		printf("ClassA\n");
	}
	~ClassB()
	{
		printf("~ClassA\n");
	}

public:
	int num_;
	int n_;
};

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

void fun(shared_ptr<ClassA> ptr)
{
	//printf("use_count=%d\n", ptr.use_count());
	ptr->num_++;

}
void fun(ClassA* ptr)
{
	ptr->num_++;

}

int main()
{

	//double sum = 0;
	//std::thread t[t_count];
	//for (int n = 0; n < t_count;n++)
	//{
	//	t[n] = std::thread(wrokFun, n);
	//}
	//CellTimeStamp timer;
	//for (int n = 0; n < t_count; n++)
	//{
	//	t[n].join();
	//}

	//std::cout << timer.get_elapsed_millisecond() << ",sum= " << sum << std::endl;
	//
	//timer.update();

	/*int* a = new int;
	*a = 11;
	shared_ptr<int> b = make_shared<int>();
	*b = 11;
	printf("a=%d, b=%d\n", *a, *b);*/

	/*ClassA* a1 = new ClassA;
	delete a1;*/
	//{
	//	shared_ptr<ClassA> b = make_shared<ClassA>(11);
	//	printf("use_count=%d\n", b.use_count());
	//	b->num_ = 11;
	//	CellTimeStamp timer;
	//	for (int n = 0; n < 10000000;n++)
	//	{
	//		 fun(b);
	//	}
	//	cout << timer.get_elapsed_millisecond() << endl;
	//	printf("use_count=%d\n", b.use_count());

	//	printf("num=%d\n", b->num_);
	//}

	//{
	//	ClassA *c = new ClassA(100);
	//	CellTimeStamp timer;
	//	for (int n = 0; n < 10000000; n++)
	//	{
	//		fun(c);
	//	}
	//	cout << timer.get_elapsed_millisecond() << endl;
	//}
	//

	ClassA* a1 = new ClassA(10);
	delete a1;

	ClassA* a2 = ClassA::create_objetc(11);
	ClassA::destroy_object(a2);

	ClassB* b1 = ClassB::create_objetc(110, 11);
	ClassB::destroy_object(b1);

	return 0;
}