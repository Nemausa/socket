#ifndef EasyTcpClient_hpp_
#define EasyTcpClient_hpp_

#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#define WIN32_LEAN_AND_MEAN  // 避免早期定义的宏冲突
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include <windows.h>
	#include <WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
	#include <unistd.h>  // unix std
	#include <arpa/inet.h>
	#include <string.h>
	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include <iostream>
using namespace std;
#include <stdio.h>
#include <thread>
#include "message.hpp"

class TcpClient
{
public:
	SOCKET sock_;

public:
	TcpClient()
	{
		sock_ = INVALID_SOCKET;
	}

	virtual ~TcpClient()
	{
		close_socket();
		
	}

	// 初始化socket
	void init_socket()
	{
		// 启动win socke 2.x环境
#ifdef _WIN32
		// 启动socket 网络环境
		WORD version = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(version, &dat);
#endif
		// 建立一个TCP客户端
		// 1.建立一个socket
		if (INVALID_SOCKET != sock_)
		{
			cout << "关闭旧的socket连接" << endl;
			close_socket();
		}
		sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		
		if (INVALID_SOCKET == sock_)
		{
			cout << "socket error" << endl;
		}
		else
			cout << "socket success" << endl;
	}

	// 连接服务器
	int connect_server(char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == sock_)
		{
			init_socket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		if (SOCKET_ERROR == connect(sock_, (sockaddr*)&_sin, sizeof(sockaddr)))
		{
			cout << "connect error" << endl;
		}
		else
			cout << "connect success" << endl;

		return 1;
	}

	// 关闭socket
	void close_socket()
	{
		if (INVALID_SOCKET == sock_)
			return;
#ifdef _WIN32
		// 4.关闭套接字
		closesocket(sock_);
		WSACleanup();
#else
		close(_sock);
#endif
		sock_ = INVALID_SOCKET;
	}


	// 处理网络消息
	bool on_run()
	{
		if (!is_run())
			return false;

		fd_set fd_read;
		FD_ZERO(&fd_read);
		FD_SET(sock_, &fd_read);

		timeval tm = { 1, 0 };
		int ret = select(sock_, &fd_read, 0, 0, &tm);
		if (ret < 0)
		{
			cout << "select任务结束1" << endl;
			return false;
		}
		if (FD_ISSET(sock_, &fd_read))
		{
			FD_CLR(sock_, &fd_read);
			if (-1 == recv_data(sock_))
			{
				cout << "select任务结束2" << endl;
				return false;
			}
		}

		return true;
	}

	// 是否运行
	bool is_run()
	{
		return INVALID_SOCKET != sock_;
	}

	// 接受数据 处理粘包 拆包
	int recv_data(SOCKET _csock)
	{
		int len_head = sizeof(DataHeader);
		// 缓冲区
		char recv_buf[1024] = {};
		// 接受客户端的请求
		int len = recv(sock_, recv_buf, sizeof(DataHeader), 0);
		DataHeader *head = (DataHeader*)recv_buf;
		if (len <= 0)
		{
			cout << "与服务器端口连接，任务结束" << endl;
			return -1;
		}
		recv(sock_, recv_buf + len_head, head->length_ - len_head, 0);
		on_net_msg(head);

		return 0;
	}

	// 响应网络消息
	void on_net_msg(DataHeader *head)
	{
		// 处理请求
		switch (head->cmd_)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult *login = (LoginResult*)head;
			cout << "收到命令:CMD_LOGIN_RESULT" << "数据长度:" << login->length_ << endl;
		}
		break;
		case CMD_SIGNOUT_RESULT:
		{
			SignOutResult *loginout = (SignOutResult*)head;
			cout << "收到命令:CMD_SIGNOUT_RESULT" << "数据长度:" << loginout->length_ << endl;
		}
		break;
		case  CMD_NEW_USER_JOIN:
		{
			NewUserJoin *user = (NewUserJoin*)head;
			cout << "收到命令:CMD_NEW_USER_JOIN" << "数据长度:" << user->length_ << endl;
		}
		break;
		default:
			DataHeader head = { CMD_ERROR, 0 };
			send(sock_, (char*)&head, sizeof(DataHeader), 0);
			break;
		}
	}

	// 发送数据
	int send_data(DataHeader *head)
	{
		if (is_run() && head)
		{
			return send(sock_, (const char*)head, head->length_, 0);
		}
		return SOCKET_ERROR;
			
	}

private:

};



#endif
