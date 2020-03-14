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
* @author morris
* contact: tappanmorris@outlook.com
*
*/

#include <iostream>
using namespace std;
#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <map>

#include "message.hpp"
#include "cell_time_stamp.hpp"
#include "cell_task.hpp"


#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#define SEND_BUFF_SIZE RECV_BUFF_SIZE
#endif

#include "cell.hpp"
#include "cell_client.hpp"
#include "net_event.hpp"
#include "cell_server.hpp"
#include "cell_net_work.hpp"


class TcpServer:public INetEvent
{
private:
	void on_run(CellThread* pthread)
	{
		while (pthread->is_run())
		{
			time_for_msg();
			fd_set fd_read;
			FD_ZERO(&fd_read);
			FD_SET(sock_, &fd_read);
			timeval t = { 0, 1 };
			int ret = select((int)sock_ + 1, &fd_read, 0, 0, &t);
			if (ret < 0)
			{
				thread_.exit();
				CellLog::warning("TcpServer.on_run  select task ends \n");
				close_socket();
			}
			if (FD_ISSET(sock_, &fd_read))
			{
				FD_CLR(sock_, &fd_read);
				accpet_client();
			}
		} 
		

		
	}
public:
	TcpServer()
	{
		sock_ = INVALID_SOCKET;
		memset(recv_buf_, 0, RECV_BUFF_SIZE);
		msg_count_ = 0;
		clients_count_ = 0;
		recv_count_ = 0;
		
	}
	virtual ~TcpServer()
	{
		close_socket();
	}


	TcpServer(const TcpServer&)
	{
		
	}
	//TcpServer& operator=(const TcpServer&) = delete;
	// 初始化socket
	SOCKET init_socket()
	{
		CellNetWork::Init();

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
		int bs = ::bind(sock_, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == bs)
		{
			CellLog::error("bind port <%d>error\n", port);
			return false;
		}
		else
		{
			CellLog::info("bind port <%d>success\n", port);
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
			CellLog::info("socket=<%d> error, listen error\n", (int)sock_);
		}
		else
		{
			CellLog::info("socket=<%d> listen success\n", (int)sock_);
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

		csock = INVALID_SOCKET;
#ifdef _WIN32
		csock = accept(sock_, (sockaddr*)&client_addr, &n_addr);
#else
		csock = accept(sock_, (sockaddr*)&client_addr, (socklen_t*)&n_addr);
#endif
		if (INVALID_SOCKET == csock)
		{
			CellLog::error("socket=<%d> error, invalid socket\n", (int)sock_);
			return INVALID_SOCKET;
		}
		else
		{
			// 将客户端分配给最小的处理线程
			add_client_to_server(new CellClient(csock));
		}
		// 获取ip地址：inet_ntoa(client_addr.sin_addr)
		return csock;
	}

	void add_client_to_server(CellClient* client)
	{
		auto min_server = cell_servers_[0];
		// 查找客户端最少的cellserver处理线程
		for (auto cell : cell_servers_)
		{
			if (min_server->get_clients_count() > cell->get_clients_count())
				min_server = cell;
		}
		min_server->addClient(client);
		//on_join(client);
	}

	void start(int servers)
	{

		for (int n = 0; n < servers; n++)
		{
			auto ser = new CellServer(n+1);
			cell_servers_.push_back(ser);
			// 注册网络事件接受对象
			ser->set_event(this);
			// 启动服务线程
			ser->start();
		}

		thread_.start(nullptr, [this](CellThread* pthread) { on_run(pthread);});
	}

	// 关闭socket
	void close_socket()
	{
		CellLog::info("Tcp_Server.close_socket 1\n");
		thread_.close();
		if (INVALID_SOCKET == sock_)
			return; 

		for (auto s : cell_servers_)
			delete s;
		cell_servers_.clear();
#ifdef _WIN32
		// 关闭套接字
		closesocket(sock_);
#else
		close(sock_);
#endif
		sock_ = INVALID_SOCKET;
		CellLog::info("Tcp_Server.close_socket 2\n");

	}


	// 网络消息计数
	void time_for_msg()
	{
		auto t = timer_.get_elapsed_second();
		if (t > 1.0)
		{
			//cout << " therad " << (int)cell_servers_.size() << ",time " << t << ",socket " << (int)sock_ << ",clients " << clients_count_ << ",msg_count " << msg_count_ << ",recv_count " << recv_count_ << endl;

			CELLLOG_DEBUG("thread<%d>,time<%lf>,socket<%d>,clients<%d>,msg_count<%d>,recv_count<%d>\n", 
				(int)cell_servers_.size(), t, (int)sock_, (int)clients_count_, (int)msg_count_, (int)recv_count_);

			timer_.update();
			msg_count_ = 0;
			recv_count_ = 0;
		}
	}



	// 多线程出发 不安全
	virtual void on_net_msg(CellServer* cell_server,CellClient* client, NetDataHeader *head)
	{
		msg_count_++;
	}

	// 多线程触发  不安全
	virtual void on_leave(CellClient* client)
	{
		if(clients_count_>0)
			clients_count_--;
	}

	// 只会被一个线程触发  安全
	virtual void on_join(CellClient* client)
	{
		clients_count_++;
	}

	virtual void on_recv(CellClient* client)
	{
		recv_count_++;
	}

private:
	char recv_buf_[RECV_BUFF_SIZE];
	// 消息处理对象，内部会创建线程
	vector<CellServer*> cell_servers_;
	CellThread thread_;
	CellTimeStamp timer_;
	SOCKET sock_;

protected:
	// 受到消息计数
	std::atomic_int msg_count_ ;
	// 客户端数量
	std::atomic_int clients_count_;
	std::atomic_int recv_count_; // recv 函数计数

};




#endif