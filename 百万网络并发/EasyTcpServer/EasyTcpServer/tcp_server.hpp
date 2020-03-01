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
* @author Morris
* contact: tappanmorris@outlook.com
*
*/

#ifdef _WIN32
	#define FD_SETSIZE      2506
	#define WIN32_LEAN_AND_MEAN
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
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <map>
#include "message.hpp"
#include "cell_time_stamp.hpp"
#include "cell_task.hpp"
using namespace std;

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240*5
#define SEND_BUFF_SIZE RECV_BUFF_SIZE
#endif



class CellServer;
// 客户端数据类型
class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		sockfd_ = sockfd;
		last_msg_pos_ = 0;
		last_send_pos_ = 0;
		memset(sz_msg_buf_, 0, RECV_BUFF_SIZE);
		memset(sz_send_buf_, 0, SEND_BUFF_SIZE);
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
		return last_msg_pos_;
	}

	void set_last_pos(int pos)
	{
		last_msg_pos_ = pos;
	}

	// 发送数据给指定的客户端
	int send_data(DataHeader *head)
	{
		int ret = SOCKET_ERROR;
		// 发送的数据长度
		int send_len = head->length_;
		// 要发送的数据
		const char* pSendData = (const char*)head;
		while (true)
		{
			if (last_send_pos_ + send_len >= SEND_BUFF_SIZE)
			{
				// 计算可以拷贝的数据长度
				int copy_len = SEND_BUFF_SIZE - last_send_pos_;
				// 拷贝数据到发送缓冲区
				memcpy(sz_send_buf_+ last_send_pos_, pSendData, copy_len);
				// 计算剩余数据位置
				pSendData += copy_len;
				// 剩余数据长度
				send_len -= copy_len;

				ret = send(sockfd_,sz_send_buf_, SEND_BUFF_SIZE, 0);
				// 数据尾部位置清零
				last_send_pos_ = 0;
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else
			{
				// 将要发送的数据拷贝到发送缓冲区尾部
				memcpy(sz_send_buf_ + last_send_pos_, pSendData, send_len);
				// 数据尾部位置
				last_send_pos_ += send_len;
				break;
			}
		}

		
		return ret;
	}

	
private:
	SOCKET sockfd_; // socket fd_set file desc set
	char sz_msg_buf_[RECV_BUFF_SIZE];  // 第二缓冲区 消息缓冲区
	char sz_send_buf_[SEND_BUFF_SIZE];  // 第二缓冲区 发送缓冲区
	int last_msg_pos_;  // 消息缓冲区的数据尾部位置
	int last_send_pos_; // 发送缓冲区尾部位置
};

// 网络事件
class INetEvent
{
public:
	// 客户端加入事件
	virtual void on_join(ClientSocket* client) = 0;
	// 客户端离开事件
	virtual void on_leave(ClientSocket* client) = 0; // 纯虚函数  继承类必须实现函数功能
	// 客户端消息事件
	virtual void on_net_msg(CellServer* cell_server,ClientSocket* client, DataHeader *head) = 0;

	virtual void on_recv(ClientSocket* client) = 0;
	
private:

};


// 网络消息发送任务
class CellSend2CellTask:public CellTask
{
public:
	CellSend2CellTask(ClientSocket *client, DataHeader* head)
	{
		client_ = client;
		head_ = head;
	}
	~CellSend2CellTask()
	{
	}

	void work()
	{
		client_->send_data(head_);
		delete head_;
	}

private:
	ClientSocket *client_;
	DataHeader *head_;

};


// 网络消息接收处理服务类
class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		sock_ = sock;
		thread_ = nullptr;
		net_event_ = nullptr;
	}
	virtual ~CellServer()
	{
		delete thread_;
		close_socket();
		sock_ = INVALID_SOCKET;
	}

	void set_event(INetEvent* net_event)
	{
		net_event_ = net_event;
	}

	// 关闭socket
	void close_socket()
	{
		if (INVALID_SOCKET == sock_)
			return;

#ifdef _WIN32
		
		for(auto iter:clients_)
		{
			closesocket(iter.second->sockfd());
			delete iter.second;
		}
		// 关闭套接字
		closesocket(sock_);
		//WSACleanup();
#else
		for (auto iter : clients_)
		{
			close(iter.second->sockfd());
			delete iter.second;
		}
		close(sock_);
#endif
		clients_.clear();
	}


	// 是否在工作中
	bool is_run()
	{
		return INVALID_SOCKET != sock_;
	}

	// 备份客户socket fd_set
	fd_set fd_read_back_;

	bool clients_change_;
	SOCKET max_socket_;
	// 处理网络消息
	void on_run()
	{
		clients_change_ = true;
		while (is_run())
		{
			// 伯克利套接字 BSD socket
			//select(
			//   _In_ int nfds, windows下无意义
			//	_Inout_opt_ fd_set FAR * readfds,		读集合
			//	_Inout_opt_ fd_set FAR * writefds,		写集合
			//	_Inout_opt_ fd_set FAR * exceptfds,		错误集合
			//	_In_opt_ const struct timeval FAR * timeout   空则阻塞下去
			//	);
			if (!clients_quene_.empty())  // 有新客户
			{
				lock_guard<mutex> lg(mutex_);
				for (auto client : clients_quene_)
					clients_[client->sockfd()] = client;
				clients_quene_.clear();
				clients_change_ = true;
			}

			if (clients_.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}


			fd_set fd_read;
			FD_ZERO(&fd_read);
			max_socket_ = clients_.begin()->second->sockfd();
			if (clients_change_)
			{
				clients_change_ = false;
				max_socket_ = clients_.begin()->second->sockfd();
				for (auto iter : clients_)
				{
					FD_SET(iter.second->sockfd(), &fd_read);
					if (iter.second->sockfd() > max_socket_)
						max_socket_ = iter.second->sockfd();
				}

				memcpy(&fd_read_back_, &fd_read, sizeof(fd_set));
				
			}
			else
			{
				memcpy(&fd_read, &fd_read_back_, sizeof(fd_set));
			}
			
			

			// nfds 是一个整数值，是指fd_set集合所有的描述符(socket)的范围，而不是数量
			// 既是所有文件描述符最大值+1，在windows中这个参数可以写0
			//timeval t = { 0, 0 };
			int ret = select((int)max_socket_ + 1, &fd_read, nullptr, nullptr, nullptr);
			if (ret < 0)
			{
				cout << "select ends" << endl;
				close_socket();
				return;
			}
			else if (ret == 0)
			{
				continue;
			}

			
#ifdef _WIN32
			for (int n = 0; n<fd_read.fd_count; n++)
			{
				auto iter = clients_.find(fd_read.fd_array[n]);
				if (iter != clients_.end())
				{
					if (-1 == recv_data(iter->second))
					{
						
						if (net_event_)
							net_event_->on_leave(iter->second);
						//delete iter->second;
						clients_.erase(iter->first);
						clients_change_ = true;
						
					}
				}
				else
				{
					printf("error,if (iter != clients_.end())");
				}
				

			}
#else

			vector<ClientSocket*> temp;
			for (auto client : temp)
			{
				clients_.erase(client->sockfd());
				delete client;
			}
			for (auto iter : clients_)
			{
				if (FD_ISSET(iter.second->sockfd(), &fd_read))
				{
					if (-1 == recv_data(iter.second))
					{
						clients_change_ = true;
						if (net_event_)
							net_event_->on_leave(iter.second);
						temp.push_back(iter.second);

					}
				}
			}

			
#endif
			
		}
	
		return;
	}

	// 接受数据 处理粘包 拆分包
	int recv_data(ClientSocket *client)
	{
		int len_head = sizeof(DataHeader);
		// 接受客户端的请求
		char *recv_buf = client->msg_buf() + client->get_last_pos();
		int len = (int)recv(client->sockfd(), recv_buf, RECV_BUFF_SIZE-client->get_last_pos(), 0);
		net_event_->on_recv(client);

		if (len <= 0)
		{
			//cout << "client:" << (int)client->sockfd() << "exited" << endl;
			return -1;
		}
		// 将收取的数据拷贝到消息缓冲区
		//memcpy(client->msg_buf() + client->get_last_pos(), recv_buf_, len);
		client->set_last_pos(client->get_last_pos() + len);
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
				net_msg(client, head);
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

	virtual void net_msg(ClientSocket* client, DataHeader *head)
	{
		net_event_->on_net_msg(this, client, head);
	}

	void start()
	{
		//thread_ = new thread(std::mem_fn(&CellServer::on_run), this);
		thread_ = new thread(&CellServer::on_run, this);
		task_server_.start();
	}

	void addClient(ClientSocket* pClient)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		clients_quene_.push_back(pClient);
	}

	unsigned int get_clients_count()
	{
		return clients_.size() + clients_quene_.size();
	}

	void add_send_task(ClientSocket* client, DataHeader *head)
	{
		CellSend2CellTask *task = new CellSend2CellTask(client, head);
		task_server_.add_task(task);
	}
private:
	SOCKET sock_;
	//char recv_buf_[RECV_BUFF_SIZE];
	// 缓冲队列的锁
	mutex mutex_;
	thread* thread_;
	// 正式客户队列
	map<SOCKET,ClientSocket*> clients_;		
	// 缓冲客户对列
	vector<ClientSocket*> clients_quene_;
	// 网络事件
	INetEvent* net_event_;
	//
	CellTaskServer task_server_;
};



class TcpServer:public INetEvent
{
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
		int bs = ::bind(sock_, (sockaddr*)&_sin, sizeof(_sin));
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
			printf("socket=<%d> error, listen error\n", (int)sock_);
		}
		else
		{
			printf("socket=<%d> listen success\n", (int)sock_);
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
			printf("socket=<%d> error, invalid socket\n", (int)sock_);
			return INVALID_SOCKET;
		}
		else
		{
			// 将客户端分配给最小的处理线程
			add_client_to_server(new ClientSocket(csock));
		}
		// 获取ip地址：inet_ntoa(client_addr.sin_addr)
		return csock;
	}

	void add_client_to_server(ClientSocket* client)
	{
		auto min_server = cell_servers_[0];
		// 查找客户端最少的cellserver处理线程
		for (auto cell : cell_servers_)
		{
			if (min_server->get_clients_count() > cell->get_clients_count())
				min_server = cell;
		}
		min_server->addClient(client);
		on_join(client);
	}

	void start(int servers)
	{

		for (int n = 0; n < servers; n++)
		{
			auto ser = new CellServer(sock_);
			cell_servers_.push_back(ser);
			// 注册网络事件接受对象
			ser->set_event(this);
			// 启动服务线程
			ser->start();
		}
	}

	// 关闭socket
	void close_socket()
	{
		if (INVALID_SOCKET == sock_)
			return;

#ifdef _WIN32
		// 关闭套接字
		closesocket(sock_);
#else
		close(sock_);
#endif

	}

	// 是否在工作中
	bool is_run()
	{
		return INVALID_SOCKET != sock_;
	}

	bool on_run()
	{
		if (!is_run())
			return false;
		time_for_msg();
		fd_set fd_read;
		FD_ZERO(&fd_read);
		FD_SET(sock_, &fd_read);
		timeval t = { 0,10 };
		int ret = select((int)sock_ + 1, &fd_read, 0, 0, &t);
		if (ret < 0)
		{
			printf("select task ends");
			close_socket();
			return false;
		}
		if (FD_ISSET(sock_, &fd_read))
		{
			FD_CLR(sock_, &fd_read);
			accpet_client();
			return true;
		}

		return true;
	}

	// 网络消息计数
	void time_for_msg()
	{
		auto t = timer_.get_elapsed_second();
		if (t > 1.0)
		{
			/*printf("thread<%d>,time<%lf>,socket<%d>,clients<%d>,msg_count<%d>,recv_count<%d> \n",
				(int)cell_servers_.size(), t, (int)sock_, clients_count_, msg_count_, (int)recv_count_);*/
			cout << " therad " << (int)cell_servers_.size() << ",time " << t << ",socket " << (int)sock_ << ",clients " << clients_count_ << ",msg_count " << msg_count_ << ",recv_count " << recv_count_ << endl;
			timer_.update();
			msg_count_ = 0;
			recv_count_ = 0;
		}
	}



	// 多线程出发 不安全
	virtual void on_net_msg(CellServer* cell_server,ClientSocket* client, DataHeader *head)
	{
		msg_count_++;
	}

	// 多线程触发  不安全
	virtual void on_leave(ClientSocket* client)
	{
		clients_count_--;
	}

	// 只会被一个线程触发  安全
	virtual void on_join(ClientSocket* client)
	{
		clients_count_++;
	}

	virtual void on_recv(ClientSocket* client)
	{
		recv_count_++;
	}

private:
	SOCKET sock_;
	// 消息处理对象，内部会创建线程
	vector<CellServer*> cell_servers_;
	char recv_buf_[RECV_BUFF_SIZE];
	CellTimeStamp timer_;
protected:
	// 受到消息计数
	atomic_int msg_count_;
	// 客户端数量
	atomic_int clients_count_;
	atomic_int recv_count_; // recv 函数计数

};




#endif