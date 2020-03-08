#ifndef CELL_SERVER_HPP_
#define CELL_SERVER_HPP_

/**
* @file cell_server.hpp
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

#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
using namespace std;

#include "cell.hpp"
#include "cell_client.hpp"
#include "net_event.hpp"
#include "cell_signal.hpp"


// 网络消息接收处理服务类
class CellServer
{
public:
	CellServer(int id)
	{
		id_ = id;
		net_event_ = nullptr;
		old_time_ = CellTime::get_time_millisecond();
		task_server_.server_id_ = id;
	}
	virtual ~CellServer()
	{
		printf("CellServer%d.~CellServer exit begin \n", id_);
		exit();
		printf("CellServer%d.~CellServer exit end \n", id_);

	}

	void set_event(INetEvent* net_event)
	{
		net_event_ = net_event;
	}

	// 关闭socket
	void exit()
	{
		printf("CellServer%d.close_socket begin\n", id_);

		if (!is_run_)
			return;

		task_server_.exit();
		is_run_ = false;
		signal_.wait();

		printf("CellServer%d.close_socket end\n", id_);


	}

	// 处理网络消息
	void on_run()
	{
		while (is_run_)
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
				{
					client->server_id = id_;
					if (net_event_)
						net_event_->on_join(client);
					clients_[client->sockfd()] = client;
				}
				clients_quene_.clear();
				clients_change_ = true;
			}

			if (clients_.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				old_time_ = CellTime::get_time_millisecond();
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
			timeval t = { 0, 1};
			int ret = select((int)max_socket_ + 1, &fd_read, nullptr, nullptr, &t);
			if (ret < 0)
			{
				cout << "select ends" << endl;
				exit();
				return;
			}
			/*else if (ret == 0)
			{
				continue;
			}*/

			read_data(fd_read);
			check_time();

		}
		printf("CellServer%d.on_run\n", id_);
		celar_clients();
		signal_.wakeup();
	}
	
	void check_time()
	{
		auto now = CellTime::get_time_millisecond();
		auto dt = now - old_time_;
		old_time_ = now;
		for (auto iter = clients_.begin(); iter != clients_.end();)
		{
			// 心跳检测
			if (iter->second->check_heart(dt))
			{
				if (net_event_)
					net_event_->on_leave(iter->second);	
				delete iter->second;
				auto iterold = iter++;
				clients_.erase(iterold);
				clients_change_ = true;
				continue;
			}
			// 定时发送检测
			iter->second->check_send(dt);
			iter++;

			
		}
	}
	void read_data(fd_set& fd_read)
	{
#ifdef _WIN32
		for (int n = 0; n < fd_read.fd_count; n++)
		{
			auto iter = clients_.find(fd_read.fd_array[n]);
			if (iter != clients_.end())
			{
				if (-1 == recv_data(iter->second))
				{

					if (net_event_)
						net_event_->on_leave(iter->second);
					delete iter->second;
					clients_.erase(iter);
					clients_change_ = true;	
				}
			}
			else
			{
				printf("error,if (iter != clients_.end())");
			}


		}
#else

		vector<CellClient*> temp;
		for (auto iter : clients_)
		{
			if (FD_ISSET(iter.second->sockfd(), &fd_read))
			{
				if (-1 == recv_data(iter.second))
				{
					clients_change_ = true;
					if (net_event_)
						net_event_->on_leave(iter.second);
					close(iter->first);
					temp.push_back(iter.second);

				}
			}
		}
		
		for (auto client : temp)
		{
			clients_.erase(client->sockfd());
			delete client;
		}

#endif
	}

	// 接受数据 处理粘包 拆分包
	int recv_data(CellClient *client)
	{
		int len_head = sizeof(NetDataHeader);
		// 接受客户端的请求
		char *recv_buf = client->msg_buf() + client->get_last_pos();
		int len = (int)recv(client->sockfd(), recv_buf, RECV_BUFF_SIZE - client->get_last_pos(), 0);
		net_event_->on_recv(client);

		if (len <= 0)
		{
			//cout << "client:" << (int)client->sockfd() << "exited" << endl;
			return -1;
		}

		// 将收取的数据拷贝到消息缓冲区
		//memcpy(client->msg_buf() + client->get_last_pos(), recv_buf_, len);
		client->set_last_pos(client->get_last_pos() + len);
		while (client->get_last_pos() >= sizeof(NetDataHeader))
		{
			// 这时候就知道了当前消息的长度
			NetDataHeader *head = (NetDataHeader*)client->msg_buf();
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

	virtual void net_msg(CellClient* client, NetDataHeader *head)
	{
		net_event_->on_net_msg(this, client, head);
	}

	void start()
	{
		if (is_run_)
			return;
		is_run_ = true;
		std::thread t(std::mem_fn(&CellServer::on_run), this);
		t.detach();
		task_server_.start();
	}

	
	void addClient(CellClient* pClient)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		clients_quene_.push_back(pClient);
	}

	unsigned int get_clients_count()
	{
		return clients_.size() + clients_quene_.size();
	}

	void add_send_task(CellClient* client, NetDataHeader *head)
	{
		//CellSend2CellTask *task = new CellSend2CellTask(client, head);
		task_server_.add_task([client, head]() {
			client->send_data(head);
			delete head;
		});
	}
private:
	void celar_clients()
	{
		for (auto iter : clients_)
			delete iter.second;

		for (auto iter : clients_quene_)
			delete iter;

		clients_quene_.clear();
		clients_.clear();
	}

private:
	
	//char recv_buf_[RECV_BUFF_SIZE];
	// 缓冲队列的锁
	
	// 正式客户队列
	map<SOCKET, CellClient*> clients_;
	// 缓冲客户对列
	vector<CellClient*> clients_quene_;
	// 备份客户socket fd_set
	fd_set fd_read_back_;
	CellTaskServer task_server_;

	// 网络事件
	INetEvent* net_event_;
	CellSignal signal_;
	mutex mutex_;
	


	SOCKET max_socket_;
	time_t old_time_;
	int id_;
	bool clients_change_ = true;
	bool is_run_ = false;	// 是否在工作中

};


#endif  // !CELL_SERVER_HPP_