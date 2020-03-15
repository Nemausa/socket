﻿#ifndef CELL_SERVER_HPP_
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
#include <map>
#include <iostream>
using namespace std;

#include "cell.hpp"
#include "cell_client.hpp"
#include "net_event.hpp"



// 网络消息接收处理服务类
class CellServer
{
public:
	CellServer(int id)
	{
		id_ = id;
		pevent_ = nullptr;
	
		task_server_.server_id_ = id;
	}
	virtual ~CellServer()
	{
		CellLog::info("CellServer%d.~CellServer exit begin ", id_);
		close();
		CellLog::info("CellServer%d.~CellServer exit end ", id_);

	}

	void set_event(INetEvent* net_event)
	{
		pevent_ = net_event;
	}

	void start()
	{
		task_server_.start();
		thread_.start(nullptr, 
			[this](CellThread* pthread) {
			on_run(pthread); 
			},
			[this](CellThread* pthread) {
				clear_clients();
			});
	}

	// 关闭socket
	void close()
	{
		CellLog::info("CellServer%d.close_socket begin", id_);
		
		task_server_.close();
		thread_.close();

		CellLog::info("CellServer%d.close_socket end", id_);


	}

	// 处理网络消息
	void on_run(CellThread* pthread)
	{
		while (pthread->is_run())
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
					clients_[client->sockfd()] = client;
					client->server_id = id_;
					if (pevent_)
						pevent_->on_join(client);
				}
				clients_quene_.clear();
				clients_change_ = true;
			}

			if (clients_.empty())
			{
				CellThread::sleep(1);
				// 旧的时间戳
				old_time_ = CellTime::get_now_millisecond();
				continue;
			}

			check_time();
			if (!do_select())
			{
				pthread->exit();
				break;
			}
			do_msg();
		}
		CellLog::info("CellServer%d.on_run", id_);
		
	}

	bool do_select()
	{
		fd_set fd_read;
		fd_set fd_write;

		max_socket_ = clients_.begin()->second->sockfd();
		if (clients_change_)
		{
			clients_change_ = false;
			FD_ZERO(&fd_read);
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


		bool need_write = false;
		FD_ZERO(&fd_write);
		for (auto iter : clients_)
		{
			// 检测需要写的客户端
			if (iter.second->need_write())
			{
				need_write = true;
				FD_SET(iter.second->sockfd(), &fd_write);
			}
		}

		//memcpy(&fd_write, &fd_read_back_, sizeof(fd_set));
		//memcpy(&fd_except, &fd_read_back_, sizeof(fd_set));



		// nfds 是一个整数值，是指fd_set集合所有的描述符(socket)的范围，而不是数量
		// 既是所有文件描述符最大值+1，在windows中这个参数可以写0
		timeval t = { 0, 1 };
		int ret;
		if (need_write)
			ret = select((int)max_socket_ + 1, &fd_read, &fd_write, nullptr, &t);
		else
			ret = select((int)max_socket_ + 1, &fd_read, nullptr, nullptr, &t);

		if (ret < 0)
		{
			CellLog::error("CellServer%d.on_run.select error ", id_);
			return false;
		}
		else if (ret == 0)
		{
			//CellThread::sleep(1);
			return true;;
		}

		read_data(fd_read);
		write_data(fd_write);

		return true;
	}


	
	void check_time()
	{
		auto now = CellTime::get_now_millisecond();
		auto dt = now - old_time_;
		old_time_ = now;
		for (auto iter = clients_.begin(); iter != clients_.end();)
		{
			// 心跳检测
			if (iter->second->check_heart(dt))
			{
				on_client_leave(iter->second);
				auto iterold = iter++;
				clients_.erase(iterold);
				continue;
			}
			// 定时发送检测
			//iter->second->check_send(dt);
			iter++;

			
		}
	}

	void on_client_leave(CellClient* pclient)
	{
		if (pevent_)
			pevent_->on_leave(pclient);
		delete pclient;
		clients_change_ = true;

	}

	
	void read_data(fd_set& fd_read)
	{
#ifdef _WIN32
		for (int n = 0; n < fd_read.fd_count; n++)
		{
			auto iter = clients_.find(fd_read.fd_array[n]);
			if (iter != clients_.end())
			{
				if (SOCKET_ERROR == recv_data(iter->second))
				{
					on_client_leave(iter->second);
					clients_.erase(iter);	
				}
			}

		}
#else

		for (auto iter = clients_.begin(); iter != clients_.end();)
		{
			if (FD_ISSET(iter->second->sockfd(), &fd_read))
			{
				if (SOCKET_ERROR == recv_data(iter->second))
				{
					on_client_leave(iter->second);
					auto iter_old = iter++;
					clients_.erase(iter_old);
					continue;
				}
			}
			iter++;
		}
	
#endif
	}

	void write_data(fd_set& fd_write)
	{

#ifdef _WIN32
		for (int n = 0; n < fd_write.fd_count; n++)
		{
			auto iter = clients_.find(fd_write.fd_array[n]);
			if (iter != clients_.end())
			{
				if (SOCKET_ERROR == iter->second->send_now())
				{
					on_client_leave(iter->second);
					clients_.erase(iter);
				}
			}



		}
#else

		for (auto iter = clients_.begin(); iter != clients_.end();)
		{
			if (iter->second->need_write() &&FD_ISSET(iter->second->sockfd(), &fd_write))
			{
				if (-1 == iter->second->send_now())
				{
					on_client_leave(iter->second);
					auto iter_old = iter++;
					clients_.erase(iter_old);
					continue;
				}
			}
			iter++;
		}

#endif
	}

	void do_msg()
	{
		CellClient* pclient = nullptr;
		for (auto iter : clients_)

			pclient = iter.second;
		// 循环 判断是否有消息需要处理
		while (pclient->has_msg())
		{
			// 处理网络消息
			on_net_msg(pclient, pclient->front_msg());
			// 移除消息队列最前面的一条数据
			pclient->pop();
		}
	}

	// 接受数据 处理粘包 拆分包
	int recv_data(CellClient *client)
	{
		int len = client->recv_data();
		/*if (len <= 0)
		{
			return SOCKET_ERROR;
		}*/
		// 触发<接收网络数据>事件
		pevent_->on_recv(client);
		return len;
	}

	virtual void on_net_msg(CellClient* client, NetDataHeader *head)
	{
		pevent_->on_net_msg(this, client, head);
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

	// void add_send_task(CellClient* client, NetDataHeader *head)
	// {
		// //CellSend2CellTask *task = new CellSend2CellTask(client, head);
		// task_server_.add_task([client, head]() {
			// client->send_data(head);
			// delete head;
		// });
	// }
private:
	void clear_clients()
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
	INetEvent* pevent_;
	mutex mutex_;
	
	SOCKET max_socket_;
	time_t old_time_;
	CellThread thread_;
	int id_;
	bool clients_change_ = true;

};


#endif  // !CELL_SERVER_HPP_