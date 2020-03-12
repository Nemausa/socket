
#ifndef _CPP_NET_100_DLL_H_
#define _CPP_NET_100_DLL_H_



extern "C" // 请按照C的方式导出
{
	int _declspec(dllexport)/*导出C++函数*/ add(int a, int b)
	{
		return a + b;
	}


}






#endif   // !_CPP_NET_100_DLL_H_