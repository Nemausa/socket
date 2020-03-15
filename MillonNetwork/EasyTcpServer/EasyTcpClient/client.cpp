#include <thread>
#include <atomic>
#include "tcp_client.hpp"
#include "cell.hpp"
#include "cell_log.hpp"
#include <iostream>
#include <vector>
#include "cell_config.hpp"
using namespace std;
#define  _CRT_SECURE_NO_WARNINGS


/*
数据会首先写入到发送缓冲区
等到socket可写时才实际发送
每个客户端在n_sendsleep时间内
最大可写入n_msg条Login消息
每条消息100字节(Login)
*/

// 服务器IP地址
const char* ip = "127.0.0.1";
// 服务器端口
uint16_t port = 4567;
// 发送线程数量
int n_thread = 1;
// 客户端数量
int n_client = 1;
// 客户端每次发几个消息
int n_msg = 1;
// 写入消息到缓冲区的间隔时间
int n_sendsleep = 1;
// 工作休眠时间
int n_worksleep = 1;
// 客户端发送缓冲区的大小
int n_send_buffer_size = SEND_BUFF_SIZE;
// 客户端接收缓冲区的大小
int n_recv_buffer_size = RECV_BUFF_SIZE;
// 检测收到服务器消息ID是否连续
bool g_run = true;

std::atomic_int send_count;
std::atomic_int read_count;
std::atomic_int n_connect;

const char ip_linux[] = "149.28.194.79";
const char ip_windows[] = "167.179.105.207";
//const char ip_local[] = "127.0.0.1";
const char ip_local[] = "192.168.1.101";


class MyClient:public TcpClient
{
public:
	MyClient()
	{
		b_checkid = CellConfig::Instance().exist_key("-check_msg_id");
	}
	// 响应网络消息
	virtual void on_net_msg(NetDataHeader *head)
	{
		// 处理请求
		switch (head->cmd_)
		{
		case CMD_LOGIN_RESULT:
		{
			NetLoginR *login = (NetLoginR*)head;
			if (b_checkid)
			{
				if (login->id_ != recv_id_)
				{
					// 当前消息与本地消息次数不匹配
					CellLog::error("on_net_msg socket<%d> id<%d> recv_id<%d>  %d", 
						pclient_->sockfd(), login->id_, recv_id_, login->id_-recv_id_);
				}
				++recv_id_;
			}
		}
		break;
		case CMD_SIGNOUT_RESULT:
		{
			NetLoginR *loginout = (NetLoginR*)head;
		}
		break;
		case  CMD_NEW_USER_JOIN:
		{
			NetNewUserJoin *user = (NetNewUserJoin*)head;
		}
		break;
		case  CMD_ERROR:
		{
			CellLog::error("socket=<%d> receive error, data length=<%d>", (int)pclient_->sockfd(), head->length_);
		}
		break;
		default:
			CellLog::warning("socket=<%d> receive unknown message", (int)pclient_->sockfd());
			break;
		}
	}

	int send_test(NetLogin* login)
	{
		int ret = 0;
		// 如果剩余发送次数大于0
		if (send_count_ > 0)
		{
			login->id_ = send_id_;
			ret = send_data(login);
			if (SOCKET_ERROR != ret)
			{
				++send_id_;
				--send_count_;
			}
		}
		return send_count_;
	}

	bool check_send(time_t dt)
	{
		reset_time_ += dt;
		// 每经过n_sendsleep毫秒
		if (reset_time_ >= n_sendsleep)
		{
			// 重置计时
			reset_time_ -= n_sendsleep;
			// 重置计数
			send_count_ = n_msg;
		}
		return send_count_>0;
	}

private:
	int recv_id_ = 1;		// 接受消息id计数
	int send_id_ = 1;		// 发送消息id计数
	time_t reset_time_ = 0;	// 发送时间计数
	int send_count_ = 0;	// 发送条数计数
	bool b_checkid = false;
};

void cmd()
{
	while (true)
	{
		char buffer[256] = {};
		scanf("%s", buffer);
		if (0 == strcmp(buffer, "exit"))
		{
			g_run = false;
			CellLog::info("exit thread");
			return;
		}
		else
		{
			CellLog::warning("不支持的命令");
		}
	}
	
}


void work_thread(CellThread* pthread, int id)
{
	// n个线程 id值为1-n
	CellLog::info("thread<%d>, start", id);
	// 客户端数量
	std::vector<MyClient*> clients(n_client);
	// 计算本线程客户端在clients中对应的index
	int begin = 0;
	int end = n_client;
	for (int n = begin; n < end; n++)
	{
		if (!pthread->is_run())
			break;
		clients[n] = new MyClient();
		// 多线程时
		CellThread::sleep(0);
	}

	for (int n = begin; n < end; n++)
	{
		if (!pthread->is_run())
			break;
		if (INVALID_SOCKET == clients[n]->init_socket(n_send_buffer_size, n_recv_buffer_size))
			break;
		if (SOCKET_ERROR == clients[n]->connect_server(ip, port))
			break;
		n_connect++;
		CellThread::sleep(0);
	}

	// 所有连接已经完成
	CellLog::info("thread<%d>, connect<begin=%d, end=%d, connect=%d>", id, begin, end, (int)n_connect);

	read_count++;
	while (read_count < n_thread && pthread->is_run())
	{
		// 等待其他的线程准备好，再发送数据
		CellThread::sleep(10);
	}

	// 消息
	NetLogin login;
	strcpy(login.username_, "morris");
	strcpy(login.passwd_, "xxxxx");
	//
	// 收发数据都是通过onrun线程
	// senddata知识将数据写入发送缓冲区
	// 等待select检测可写时才发送数据
	auto t2 = CellTime::get_time_millisecond();
	auto t0 = t2;
	auto dt = t0;

	CellTimeStamp timer;
	while (pthread->is_run())
	{
		for (int n = begin; n < end; n++)
		{
			if (SOCKET_ERROR != clients[n]->send_data(&login))
			{
				++send_count;
			}
			else
			{
				cout << "send error";
			}
			clients[n]->on_run(1);
		}

	}
	//while (pthread->is_run())
	//{
	//	t0 = CellTime::get_time_millisecond();
	//	dt = t0 - t2;
	//	t2 = t0;

	//	int count = 0;
	//	// 每轮每个客户端发送n_msg条数据
	//	for (int m = 0; m < n_msg; m++)
	//	{
	//		for (int n = begin; n < end; n++)
	//		{
	//			// 每个客户端一条一条的写入
	//			if (clients[n]->is_run())
	//			{
	//				if (clients[n]->send_test(&login)>0)
	//					++send_count;
	//			}
	//		}
	//	}


	//	for (int n = begin; n < end; n++)
	//	{
	//		if (clients[n]->is_run())
	//		{
	//			if (!clients[n]->on_run(0))
	//			{
	//				n_connect--;
	//				continue;
	//			}
	//			// 检测发送计数是否要重置
	//			clients[n]->check_send(dt);
	//		}

	//	}

	//	CellThread::sleep(n_worksleep);

	//}


}


int main(int argc, char* args[])
{
	
	CellLog::Instance().set_path("client_log.txt", "w", false);
	CellConfig::Instance().init(argc, args);

	ip = CellConfig::Instance().get_str("ip", "127.0.0.1");
	port = CellConfig::Instance().get_int("port", 4567);
	n_thread = CellConfig::Instance().get_int("n_thread", 1);
	n_client = CellConfig::Instance().get_int("n_client", 10);
	n_msg = CellConfig::Instance().get_int("n_msg", 100);
	n_sendsleep = CellConfig::Instance().get_int("n_sendsleep", 1);
	n_worksleep = CellConfig::Instance().get_int("n_worksleep", 1);
	n_send_buffer_size = CellConfig::Instance().get_int("n_send_buffer_size", SEND_BUFF_SIZE);
	n_recv_buffer_size = CellConfig::Instance().get_int("n_recv_buffer_size", RECV_BUFF_SIZE);

	// 启动终端命令线程
	// 用于接受运行时用户输出的命令
	CellThread tcmd;
	tcmd.start(nullptr, [=](CellThread* pthread) {
		cmd();
	});

	// 启动模拟客户端线程
	vector<CellThread*> threads;
	for (int n = 0; n < n_thread; n++)
	{
		CellThread* t = new CellThread();
		t->start(nullptr, [n](CellThread* pthread) {
			work_thread(pthread, n + 1);
		});
		threads.push_back(t);
	}



	// 每秒数据统计
	CellTimeStamp timer;
	while (tcmd.is_run())
	{
		auto t = timer.get_elapsed_second();
		if (t >= 1.0)
		{
			CellLog::info("thread<%d>,clients<%d>,time<%lf>,send<%d>",n_thread, (int)n_connect, t, (int)send_count);
			send_count = 0;
			timer.update();
		}
		CellThread::sleep(1);
	}

	for (auto t : threads)
	{
		t->close();
		delete t;
	}
		

	CellLog::info("...exit...");
	return 0;
}


