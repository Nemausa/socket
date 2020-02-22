#ifndef TCPCLIENT_HPP_
#define TCPCLIENT_HPP_

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
			cout << "close old socket" << endl;
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
	int connect_server(const char* ip, unsigned short port)
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
			cout << "socket=" << sock_ <<"ip" << ip <<"port="<< port << "connect error" << endl;
		}
		else
			cout << "socket=" << sock_ <<"ip" << ip << "port=" << port << "connect success" << endl;

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
		close(sock_);
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

		timeval tm = { 0, 0 };
		int ret = select(sock_, &fd_read, 0, 0, &tm);
		if (ret < 0)
		{
			cout << "select taks ends 1" << endl;
			close_socket();
			return false;
		}
		if (FD_ISSET(sock_, &fd_read))
		{
			FD_CLR(sock_, &fd_read);
			if (-1 == recv_data(sock_))
			{
				cout << "select task ends 2" << endl;
				close_socket();
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
			cout << "the task ends" << endl;
			return -1;
		}
		recv(sock_, recv_buf + len_head, head->length_ - len_head, 0);
		on_net_msg(head);

		return 0;
	}

	// 响应网络消息
	virtual void on_net_msg(DataHeader *head)
	{
		// 处理请求
		switch (head->cmd_)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult *login = (LoginResult*)head;
			cout << "socket=" << sock_ << "command:CMD_LOGIN_RESULT" << "data length:" << login->length_ << endl;
		}
		break;
		case CMD_SIGNOUT_RESULT:
		{
			SignOutResult *loginout = (SignOutResult*)head;
			cout << "socket=" << sock_ << "command:CMD_SIGNOUT_RESULT" << "data length:" << loginout->length_ << endl;
		}
		break;
		case  CMD_NEW_USER_JOIN:
		{
			NewUserJoin *user = (NewUserJoin*)head;
			cout << "socket=" << sock_ << "command:CMD_NEW_USER_JOIN" << "data length:" << user->length_ << endl;
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
