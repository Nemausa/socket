﻿#include <iostream>
#include <vector>
using namespace std;
#include "tcp_server.hpp"
#include <thread>

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

CellTimeStamp timer;
auto t = timer.get_elapsed_second();

class MyServer :public TcpServer
{
public:
	// 多线程出发 不安全
	virtual void on_net_msg(CellServer* cell_server, CellClient* client, NetDataHeader *head)
	{
		TcpServer::on_net_msg(cell_server, client, head);
		switch (head->cmd_)
		{
		case CMD_LOGIN:
		{
			client->reset_heart();
			NetLogin *login = (NetLogin*)head;
			//printf("command CMD_LOGIN socket=<%d> data length=<%d> username=<%s> passwd=<%s>\n", (int)csock, login->length_, login->username_, login->passwd_);
			// 判断用户密码正确的过程
			NetLoginR ret;
			if (0 == client->send_data(&ret))
			{
				// 发送缓冲区满了，消息还没有发送出去
				//printf("<socket=%d> send full \n", client->sockfd());
			}
			// 接收-消息 ----处理发送   生产者 数据缓冲区  消费者
			/*NetLoginR* ret = new NetLoginR();
			cell_server->add_send_task(client, ret);*/
		}
		break;
		case CMD_SIGNOUT:
		{

			//printf("command CMD_SIGNOUT socket=<%d> data length=<%d> username=<%s>\n", (int)csock, head->length_, loginout->username_);
			// 判断用户密码正确的过程
			//SignOutResult ret = {};
			//send_data(csock, &ret);
		}
		break;
		case CMD_HEART_C2S:
		{
			client->reset_heart();
			Net_C2S_Heart ret;
			client->send_data(&ret);
		}
		break;
		default:
			NetDataHeader ret = {};
			//send_data(csock, &ret);
			printf("command CMD_ERROR socket=<%d> data length=<%d>\n", (int)client->sockfd(), ret.length_);
			break;
		}
	}

	// 多线程触发  不安全
	virtual void on_leave(CellClient* client)
	{
		TcpServer::on_leave(client);

	}

	// 只会被一个线程触发  安全
	virtual void on_join(CellClient* client)
	{
		TcpServer::on_join(client);
	}

	virtual void on_recv(CellClient* client)
	{
		TcpServer::on_recv(client);
	}

private:

};





int main()
{
	MyServer server1;
	server1.init_socket();
	server1.bind_port(nullptr, 4567);
	server1.listen_port(5);
	server1.start(1);


	while (true)
	{
		char buffer[256] = {};
		scanf("%s", buffer);
		if (0 == strcmp(buffer, "exit"))
		{
			server1.close_socket();
			cout << "退出线程" << endl;
			break;;
		}
		else
		{
			cout << "不支持的命令" << endl;
		}
	}
	
	//CellTaskServer task;
	//task.start();
	//Sleep(100);
	//task.exit();

	while (true)
	{
		std::chrono::microseconds dura(1);
		std::this_thread::sleep_for(dura);
	}
	printf("已退出\n");
	return 0;
}

