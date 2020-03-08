﻿#include <thread>
#include <atomic>
#include "tcp_client.hpp"
#include "cell_time_stamp.hpp"
#define  _CRT_SECURE_NO_WARNINGS

bool g_run = true;

void cmd()
{
	while (true)
	{
		char buffer[256] = {};
		scanf("%s", buffer);
		if (0 == strcmp(buffer, "exit"))
		{
			g_run = false;
			cout << "退出线程" << endl;
			return;
		}
		else
		{
			cout << "不支持的命令" << endl;
		}
	}
	
}

const int t_count = 1;    // 线程数量
const int c_count = 10; // 客户端数量
TcpClient *client[c_count];
std::atomic_int send_count;
std::atomic_int read_count;

void recv_thread(int begin, int end)
{
	CellTimeStamp t;
	while (g_run)
	{
		for (int n = begin; n < end; n++)
		{
			/*if(t.get_elapsed_second()>3.0 && n== begin)
				continue;*/

			client[n]->on_run();
		}
	}

}

void send_thread(int id)
{
	int c = c_count / t_count;
	int begin = (id - 1)*c;
	int end = id*c;
	// linux	149.28.194.79
	// windows	167.179.105.207
	const char ip_linux[] = "149.28.194.79";
	const char ip_windows[] = "167.179.105.207";
	const char ip_local[] = "127.0.0.1";
	//const char ip_local[] = "192.168.1.101";

	for (int n = begin; n < end; n++)
	{
		client[n] = new TcpClient();
	}
	for (int n = begin; n < end; n++)
	{
		client[n]->init_socket();
		client[n]->connect_server(ip_linux, 4567);
		
	}
	printf("thread<%d>,connect<beging=%d,end=%d>\n", id, begin, end);
	
	read_count++;
	while (read_count < t_count)
	{
		// 等待其他线程并发发送数据
		std::chrono::milliseconds t(100);
		std::this_thread::sleep_for(t);
	}

	NetLogin login[10];
	for (int n = 0; n < 10; n++)
	{
		strcpy(login[n].passwd_, "passwd");
		strcpy(login[n].username_, "morris");
	}
	std::thread t1(recv_thread, begin, end);
	t1.detach();

	const int len = sizeof(login);

	while (g_run)
	{
		for (int n = begin; n < end; n++)
			if (SOCKET_ERROR != client[n]->send_data(login, len))
				send_count++;	
			
		
		//std::chrono::milliseconds t(1000);
		//std::this_thread::sleep_for(t);
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->close_socket();
		delete client[n];
	}
	printf("thread<%d>,exit<beging=%d,end=%d>\n", id, begin, end);
}



int main()
{
	

	// UI线程
	thread cmd_thread(cmd);
	cmd_thread.detach();

	
	// 启动发送线程
	for (int n = 0; n < t_count; n++)
	{
		thread t1(send_thread, n+1);
		t1.detach();
	}

	CellTimeStamp timer;
	while (g_run)
	{
		auto t = timer.get_elapsed_second();
		if (t > 1.0)
		{
			printf("thread<%d>,clients<%d>,time<%lf>,send_count<%d>\n", t_count, c_count, t, (int)(send_count/t));
			send_count = 0;
			timer.update();
		}
		
	}

	
	std::chrono::milliseconds t(100);
	while (g_run)
		std::this_thread::sleep_for(t);

	cout << "exited" << endl;

	
	return 0;
}

