
#ifndef _CPP_NET_100_DLL_H_
#define _CPP_NET_100_DLL_H_
#include <string>


extern "C" // 请按照C的方式导出
{
	int _declspec(dllexport)/*导出C++函数*/ add(int a, int b)
	{
		return a + b;
	}

	typedef void(*callback1)(const char* str);

	void _declspec(dllexport) TestCall(const char* str1, callback1 cb)
	{
		std::string s = "hello ";
		s += str1;
		cb(s.c_str());
	}

}






#endif   // !_CPP_NET_100_DLL_H_