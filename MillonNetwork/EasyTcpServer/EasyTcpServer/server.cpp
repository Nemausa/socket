#include <iostream>
#include <vector>
#include <thread>
using namespace std;
#include "tcp_server.hpp"
#include "cell_msg_stream.hpp"

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
			CellLog::info("退出线程\n");
			return;
		}
		else
		{
			CellLog::info("不支持的命令\n");
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
			//CellLog::info("command CMD_LOGIN socket=<%d> data length=<%d> username=<%s> passwd=<%s>\n", (int)csock, login->length_, login->username_, login->passwd_);
			// 判断用户密码正确的过程
			NetLoginR ret;
			if (0 == client->send_data(&ret))
			{
				//发送缓冲区满了，消息还没有发送出去
				CellLog::warning("<socket=%d> send full \n", client->sockfd());
				//CellLog::info("<socket=%d> send full \n", client->sockfd());
			}
			// 接收-消息 ----处理发送   生产者 数据缓冲区  消费者
			/*NetLoginR* ret = new NetLoginR();
			cell_server->add_send_task(client, ret);*/
		}
		break;
		case CMD_SIGNOUT:
		{
			CellReadStream r(head);
			// 预读取长度
			r.read_int16();
			r.get_cmd();
			auto n1 = r.read_int8();
			auto n2 = r.read_int16();
			auto n3 = r.read_int32();
			auto n4 = r.read_float();
			auto n5 = r.read_double();
			char name[32] = {};
			auto n6 = r.read_array(name, 32);
			char pw[32] = {};
			auto n7 = r.read_array(pw, 32);
			int ata[10] = {};
			auto n8 = r.read_array(ata, 10);

			CellWriteStream s(128);
			s.set_cmd(CMD_SIGNOUT_RESULT);
			s.write_int8(5);
			s.write_int16(4);
			s.write_int32(3);
			s.write_float(2.0f);
			s.write_double(1.6);

			char* str = "server";
			s.write_array(str, strlen(str));
			char a[] = "passwd";
			s.write_array(a, strlen(a));
			int b[] = { 1,2,3,4,5 };
			s.write_array(b, 5);
			s.finish();
			client->send_data(s.data(), s.length());
			
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
			CellLog::error("command CMD_ERROR socket=<%d> data length=<%d>\n", (int)client->sockfd(), ret.length_);
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

const char* arg_to_str(int argc, char* args[], int index, const char* def, const char* name)
{
	if (index >= argc)
	{
		CellLog::error("argtostr, index=%d, argc=%d, name=%s", index, argc, name);
	}
	else
	{
		def = args[index];
	}
	CellLog::info("%s=%s", name, def);
	return def;
}

int arg_to_int(int argc, char* args[], int index, int def, const char* name)
{
	if (index >= argc)
	{
		CellLog::error("arg_to_int, index=%d, argc=%d, name=%s", index, argc, name);
	}
	else
	{
		def = atoi(args[index]);
	}
	CellLog::info("%s=%d", name, def);
	return def;
}


int main(int argc, char* args[])
{

	const char* ip = arg_to_str(argc, args, 1, "any", "ip");
	uint16_t port = arg_to_int(argc, args, 2, 4567, "port");
	int n_thread = arg_to_int(argc, args, 3, 1, "thread");
	int n_client = arg_to_int(argc, args, 4, 1, "client");

	if (strcmp(ip, "any") == 0)
		ip = nullptr;

	
	CellLog::Instance().set_path("server_log", "w");
	MyServer server1;
	server1.init_socket();
	server1.bind_port(ip, port);
	server1.listen_port(5);
	server1.start(n_thread);


	while (true)
	{
		char buffer[256] = {};
		scanf("%s", buffer);
		if (0 == strcmp(buffer, "exit"))
		{
			CellLog::info("退出线程\n");
			break;;
		}
		else
		{
			CellLog::warning("不支持的命令\n");
		}
		std::chrono::microseconds dura(1);
		std::this_thread::sleep_for(dura);
	}
	
	//CellTaskServer task;
	//task.start();
	//Sleep(100);
	//task.exit();

	//while (true)
	//{
	//	std::chrono::microseconds dura(1);
	//	std::this_thread::sleep_for(dura);
	//}
	CellLog::info("已退出\n");
	return 0;
}

