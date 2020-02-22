#ifndef TCP_SERVER_HPP_
#define TCP_SERVER_HPP_

/**
* @file tcp_server.hpp
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-02-22
* @author Kevin
* contact: tappanmorris@outlook.com
*
*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN  // 避免早期定义的宏冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else//#elif __APPLE__
#include <unistd.h>  // unix std
#include <arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

//#else
//#   error "Unknown compiler"
#endif

#include <iostream>
#include <stdio.h>
#include <vector>
#include "message.hpp"
using namespace std;



class TcpServer
{
private:
	SOCKET sock_;
	vector<SOCKET> g_clients_;
public:
	TcpServer()
	{
		sock_ = INVALID_SOCKET;
	}
	virtual ~TcpServer()
	{
		close_socket();
	}

	// 初始化socket
	SOCKET init_socket()
	{
#ifdef _WIN32
		// 启动socket 网络环境
		WORD version = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(version, &dat);
#endif
		if (INVALID_SOCKET != sock_)
		{
			cout << "close an existing socket:" << sock_ << endl;
			close_socket();
		}
		// 建立一个TCP服务器
		// 1.建立一个socket
		sock_ = socket(AF_INET/*IPV4*/, SOCK_STREAM/*数据流*/, IPPROTO_TCP);
		if (INVALID_SOCKET == sock_)
		{
			cout << "failed to create socket" << endl;
		}
		else
		{
			cout << "socket created successfully" << endl;
		}
		return sock_;
	}

	// 绑定端口
	int bind_port(const char* ip, unsigned int port)
	{
		if (INVALID_SOCKET == sock_)
			init_socket();
		// 2.bind 绑定用于接受客户端的网络端口
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);


		unsigned long ip_s = ip ? inet_addr(ip): INADDR_ANY;

#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = ip_s; //inet_addr("127.0.0.1");
#else
		_sin.sin_addr.s_addr = ip_s; //inet_addr("127.0.0.1");
#endif
		int bs = bind(sock_, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == bs)
		{
			printf("error, bind port <%d>error\n", port);
			return false;
		}
		else
		{
			printf("bind port <%d>success\n", port);
		}
		return true;
	}

	// 监听端口
	int listen_port(int count)
	{
		// 3.监听网络端口
		int result =listen(sock_, count);
		if (SOCKET_ERROR == result)
		{
			printf("socket=<%d> error, listen error\n", sock_);
		}
		else
		{
			printf("socket=<%d> listen success\n", sock_);
		}

		return result;
	}

	// 接受客户端连接
	SOCKET accpet_client()
	{
		// 4.等待客户端连接
		sockaddr_in client_addr = {};
		SOCKET _csock = INVALID_SOCKET;
		int n_addr = sizeof(sockaddr_in);

#ifdef _WIN32
		_csock = accept(sock_, (sockaddr*)&client_addr, &n_addr);
#else
		_csock = accept(sock_, (sockaddr*)&client_addr, (socklen_t*)&n_addr);
#endif
		if (INVALID_SOCKET == sock_)
		{
			printf("socket=<%d> error, invalid socket\n", sock_);
			return INVALID_SOCKET;
		}
		
		NewUserJoin user = {};
		send_to_all(&user);

		printf("socket=<%d>, client with socket=<%d> and ip=<%s> joined\n", sock_, _csock, inet_ntoa(client_addr.sin_addr));
		g_clients_.push_back(_csock);
		return _csock;
	}

	// 关闭socket
	void close_socket()
	{
		if (INVALID_SOCKET == sock_)
			return;

#ifdef _WIN32
		for (int n = (int)g_clients_.size() - 1; n >= 0; n--)
		{
			closesocket(g_clients_[n]);
		}
		// 关闭套接字
		closesocket(sock_);
		WSACleanup();
#else
		for (int n = (int)g_clients_.size() - 1; n >= 0; n--)
		{
			close(g_clients_[n]);
		}
		close(sock_);
#endif
	}

	// 处理网络消息
	bool on_run()
	{
		if (!is_run())
			return false;
		// 伯克利套接字 BSD socket
		//select(
		//   _In_ int nfds, windows下无意义
		//	_Inout_opt_ fd_set FAR * readfds,		读集合
		//	_Inout_opt_ fd_set FAR * writefds,		写集合
		//	_Inout_opt_ fd_set FAR * exceptfds,		错误集合
		//	_In_opt_ const struct timeval FAR * timeout   空则阻塞下去
		//	);
		fd_set fd_read;
		fd_set fd_write;
		fd_set fd_except;

		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_except);

		FD_SET(sock_, &fd_read);
		FD_SET(sock_, &fd_write);
		FD_SET(sock_, &fd_except);

		SOCKET max_socket = sock_;
		for (int n = (int)g_clients_.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients_[n], &fd_read);
			if (max_socket < g_clients_[n])
				max_socket = g_clients_[n];
		}

		// nfds 是一个整数值，是指fd_set集合所有的描述符(socket)的范围，而不是数量
		// 既是所有文件描述符最大值+1，在windows中这个参数可以写0
		timeval tm = { 1, 0 };
		int ret = select(max_socket + 1, &fd_read, &fd_write, &fd_except, &tm);
		if (ret < 0)
		{
			cout << "select ends" << endl;
			return false;
		}
		if (FD_ISSET(sock_, &fd_read))  // 判断集合是否有可操作的socket
		{
			FD_CLR(sock_, &fd_read);
			accpet_client();
		}

		//for (size_t n = 0; n < fd_read.fd_count; n++)
		//{
		//	if (-1 == process(fd_read.fd_array[n]))
		//	{
		//		auto iter = find(g_clients.begin(), g_clients.end(), fd_read.fd_array[n]);
		//		if (iter != g_clients.end())
		//			g_clients.erase(iter);
		//	}
		//}
		for (int n = (int)g_clients_.size() - 1; n >= 0; n--)
		{
			if (FD_ISSET(g_clients_[n], &fd_read))
			{
				if (-1 == recv_data(g_clients_[n]))
				{
					auto iter = g_clients_.begin() + n;
					if (iter != g_clients_.end())
						g_clients_.erase(iter);

				}
			}
		}

		return true;
	}
	// 是否在工作中
	bool is_run()
	{
		return INVALID_SOCKET != sock_;
	}
	// 接受数据 处理粘包 拆分包
	int recv_data(SOCKET _csock)
	{
		int len_head = sizeof(DataHeader);
		// 缓冲区
		char recv_buf[1024] = {};
		// 接受客户端的请求
		int len = (int)recv(_csock, recv_buf, sizeof(DataHeader), 0);
		DataHeader *head = (DataHeader*)recv_buf;
		if (len <= 0)
		{
			cout << "client:" << (int)_csock << "exited" << endl;
			return -1;
		}
		recv(_csock, recv_buf + len_head, head->length_ - len_head, 0);
		on_net_msg(_csock, head);
		return 0;
	}

	// 响应网络消息
	virtual void on_net_msg(SOCKET _csock, DataHeader *head)
	{
		// 处理请求
		switch (head->cmd_)
		{
		case CMD_LOGIN:
		{
			
			Login *login = (Login*)head;
			printf("command CMD_LOGIN socket=<%d> data length=<%d> username=<%s> passwd=<%s>\n", (int)_csock, login->length_, login->username_, login->passwd_);
			// 判断用户密码正确的过程
			LoginResult ret;
			send(_csock, (char*)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_SIGNOUT:
		{
			
			SignOut *loginout = (SignOut*)head;
			cout << "command CMD_SIGNOUT " << "socket:" << _csock << " data length :" << loginout->length_ << " userName:" << loginout->username_ << endl;
			printf("command CMD_LOGIN socket=<%d> data length=<%d> username=<%s>\n", (int)_csock, head->length_, loginout->username_);
			// 判断用户密码正确的过程
			SignOutResult ret = {};
			send(_csock, (char*)&ret, sizeof(SignOutResult), 0);
		}
		break;
		default:
			DataHeader head = { CMD_ERROR, 0 };
			send(_csock, (char*)&head, sizeof(DataHeader), 0);
			break;
		}

	}
	// 发送数据给指定的客户端
	int send_data(SOCKET _csock, DataHeader *head)
	{
		if (is_run() && head)
		{
			return send(_csock, (const char*)head, head->length_, 0);
		}
		return SOCKET_ERROR;
	}

	// 发送数据给所有的客户端
	void send_to_all(DataHeader *head)
	{
		for (int n = (int)g_clients_.size() - 1; n >= 0; n--)
		{
			send_data(g_clients_[n], head);
		}
		
	}

private:

};


#endif