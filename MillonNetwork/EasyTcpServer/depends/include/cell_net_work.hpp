#ifndef CELL_NET_WORK_HPP_
#define CELL_NET_WORK_HPP_

/**
* @file cell_net_work.hpp
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-03-09
* @author morris
* contact: tappanmorris@outlook.com
*
*/
#include "cell.hpp"
class CellNetWork
{
private:
	CellNetWork()
	{
#ifdef _WIN32
		// 启动socket 网络环境
		WORD version = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(version, &dat);
#else
		/*if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return 1;*/
		// 忽略异常情况，默认会导致进程退出
		signal(SIGPIPE, SIG_IGN);
#endif

	}
	~CellNetWork()
	{
#ifdef _WIN32
		WSACleanup();
#endif // _WIN32

	}
public:
	static void Init()
	{
		static CellNetWork netowrk;
	}

};


#endif  // ! CELL_NET_WORK_HPP_