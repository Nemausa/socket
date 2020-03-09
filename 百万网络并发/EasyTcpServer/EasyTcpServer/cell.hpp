#ifndef CELL_HPP_
#define CELL_HPP_

/**
* @file cell.hpp
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-03-06
* @author morris
* contact: tappanmorris@outlook.com
*
*/

#ifdef _WIN32
#define FD_SETSIZE      1024
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else//#elif __APPLE__
#include <unistd.h>  // unix std
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

//#else
//#   error "Unknown compiler"
#endif



// 自定义
#include "message.hpp"
#include "cell_time_stamp.hpp"
#include "cell_task.hpp"
#include "cell_thread.hpp"
#include "cell_log.hpp"
#include <stdio.h>

// 缓冲区大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#define SEND_BUFF_SIZE RECV_BUFF_SIZE
#endif

#endif  // !CELL_HPP_