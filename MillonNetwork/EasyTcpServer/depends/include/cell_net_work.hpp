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
		static CellNetWork sky;
	}

	static int make_nonblocking(SOCKET fd)
	{
#ifdef _WIN32
	{
		unsigned long nonblocking = 1;
		if (ioctlsocket(fd, FIONBIO, &nonblocking) == SOCKET_ERROR) {
			CELLLOG_WARN("fcntl(%d, F_GETFL)", (int)fd);
			return -1;
		}
	}
#else
	{
		int flags;
		if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
			CELLLOG_WARN("fcntl(%d, F_GETFL)", fd);
			return -1;
		}
		if (!(flags & O_NONBLOCK)) {
			if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
				CELLLOG_WARN("fcntl(%d, F_SETFL)", fd);
				return -1;
			}
		}
	}
#endif
	return 0;
	}

	static int make_reuseaddr(SOCKET fd)
	{
		int flag = 1;
		if (SOCKET_ERROR == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag))) {
			CELLLOG_WARN("setsockopt socket<%d> SO_REUSEADDR fail", (int)fd);
			return SOCKET_ERROR;
		}
		return 0;
	}
};


#endif  // ! CELL_NET_WORK_HPP_