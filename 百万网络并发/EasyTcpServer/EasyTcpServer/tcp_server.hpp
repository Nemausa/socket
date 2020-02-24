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
	#define FD_SETSIZE 4800
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

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif



class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		sockfd_ = sockfd;
		last_pos_ = 0;
		memset(sz_msg_buf_, 0, RECV_BUFF_SIZE * 10);
	}
	virtual ~ClientSocket()
	{

	}

	SOCKET sockfd()
	{
		return sockfd_;
	}

	char *msg_buf()
	{
		return sz_msg_buf_;
	}

	int get_last_pos()
	{
		return last_pos_;
	}

	void set_last_pos(int pos)
	{
		last_pos_ = pos;
	}

	
private:
	SOCKET sockfd_; // socket fd_set file desc set
	// 第二缓冲区 消息缓冲区
	char sz_msg_buf_[RECV_BUFF_SIZE * 10];
	// 消息缓冲区的数据尾部位置
	int last_pos_;
};

class TcpServer
{
private:
	SOCKET sock_;
	vector<ClientSocket*> clients_;
	char recv_buf_[RECV_BUFF_SIZE];
public:
	TcpServer()
	{
		sock_ = INVALID_SOCKET;
		memset(recv_buf_, 0, RECV_BUFF_SIZE);
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
		SOCKET csock = INVALID_SOCKET;
		int n_addr = sizeof(sockaddr_in);

#ifdef _WIN32
		csock = accept(sock_, (sockaddr*)&client_addr, &n_addr);
#else
		csock = accept(sock_, (sockaddr*)&client_addr, (socklen_t*)&n_addr);
#endif
		if (INVALID_SOCKET == sock_)
		{
			printf("socket=<%d> error, invalid socket\n", sock_);
			return INVALID_SOCKET;
		}
		
		//NewUserJoin user = {};
		//send_to_all(&user);

		clients_.push_back(new ClientSocket(csock));
		printf("socket=<%d>, client with socket=<%d> and ip=<%s> joined, clients size=<%d>\n", sock_, csock, inet_ntoa(client_addr.sin_addr), clients_.size());
		return csock;
	}

	// 关闭socket
	void close_socket()
	{
		if (INVALID_SOCKET == sock_)
			return;

#ifdef _WIN32
		for (int n = (int)clients_.size() - 1; n >= 0; n--)
		{
			closesocket(clients_[n]->sockfd());
			delete clients_[n];
		}
		// 关闭套接字
		closesocket(sock_);
		WSACleanup();
#else
		for (int n = (int)clients_.size() - 1; n >= 0; n--)
		{
			close(clients_[n]->sockfd());
			delete clients_[n];
		}
		close(sock_);
#endif
		clients_.clear();
	}

	int count_ = 0;
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
		for (int n = (int)clients_.size() - 1; n >= 0; n--)
		{
			FD_SET(clients_[n]->sockfd(), &fd_read);
			if (max_socket < clients_[n]->sockfd())
				max_socket = clients_[n]->sockfd();
		}

		// nfds 是一个整数值，是指fd_set集合所有的描述符(socket)的范围，而不是数量
		// 既是所有文件描述符最大值+1，在windows中这个参数可以写0
		timeval tm = { 1, 0 };
		int ret = select(max_socket + 1, &fd_read, &fd_write, &fd_except, &tm);
		//printf("select ret=<%d> count=<%d>\n", ret, count_++);
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
		for (int n = (int)clients_.size() - 1; n >= 0; n--)
		{
			if (FD_ISSET(clients_[n]->sockfd(), &fd_read))
			{
				if (-1 == recv_data(clients_[n]))
				{
					auto iter = clients_.begin() + n;
					if (iter != clients_.end())
					{
						delete clients_[n];
						clients_.erase(iter);
					}

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
	int recv_data(ClientSocket *client)
	{
		int len_head = sizeof(DataHeader);
		// 接受客户端的请求
		int len = (int)recv(client->sockfd(), recv_buf_, 4096, 0);

		if (len <= 0)
		{
			cout << "client:" << (int)client->sockfd() << "exited" << endl;
			return -1;
		}
		// 将收取的数据拷贝到消息缓冲区
		memcpy(client->msg_buf() + client->get_last_pos(), recv_buf_, len);
		client->set_last_pos(client->get_last_pos()+len);
		while (client->get_last_pos() >= sizeof(DataHeader))
		{
			// 这时候就知道了当前消息的长度
			DataHeader *head = (DataHeader*)client->msg_buf();
			// 判断消息缓冲区的数据长度大于消息长度
			if (client->get_last_pos() >= head->length_)
			{
				// 消息缓冲区剩余未处理的数据
				int size = client->get_last_pos() - head->length_;
				// 处理网络消息
				on_net_msg(client->sockfd(), head);
				// 将消息缓冲区剩余未处理的数据迁移
				memcpy(client->msg_buf(), client->msg_buf() + head->length_, size);
				// 消息缓冲区的尾部位置前移
				client->set_last_pos(size);

			}
			else
			{
				// 消息缓冲区剩余的数据不够一条消息
				break;
			}
		}

		return 0;
	}

	// 响应网络消息
	virtual void on_net_msg(SOCKET csock, DataHeader *head)
	{
		// 处理请求
		switch (head->cmd_)
		{
		case CMD_LOGIN:
		{
			
			Login *login = (Login*)head;
			//printf("command CMD_LOGIN socket=<%d> data length=<%d> username=<%s> passwd=<%s>\n", (int)csock, login->length_, login->username_, login->passwd_);
			// 判断用户密码正确的过程
			//LoginResult ret;
			//send_data(csock, &ret);
		}
		break;
		case CMD_SIGNOUT:
		{
			
			SignOut *loginout = (SignOut*)head;
			//printf("command CMD_SIGNOUT socket=<%d> data length=<%d> username=<%s>\n", (int)csock, head->length_, loginout->username_);
			// 判断用户密码正确的过程
			//SignOutResult ret = {};
			//send_data(csock, &ret);
		}
		break;
		default:
			DataHeader ret = {};
			//send_data(csock, &ret);
			printf("command CMD_ERROR socket=<%d> data length=<%d>\n", (int)csock, ret.length_);
			break;
		}

	}
	// 发送数据给指定的客户端
	int send_data(SOCKET csock, DataHeader *head)
	{
		if (is_run() && head)
		{
			return send(csock, (const char*)head, head->length_, 0);
		}
		return SOCKET_ERROR;
	}

	// 发送数据给所有的客户端
	void send_to_all(DataHeader *head)
	{
		for (int n = (int)clients_.size() - 1; n >= 0; n--)
		{
			send_data(clients_[n]->sockfd(), head);
		}
		
	}

private:

};


#endif