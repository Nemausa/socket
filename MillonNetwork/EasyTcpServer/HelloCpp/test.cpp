#include <thread>
#include <atomic>
#include "tcp_client.hpp"
#include "cell.hpp"
#include "cell_log.hpp"
#include "cell_stream.hpp"
#include <iostream>
using namespace std;
#define  _CRT_SECURE_NO_WARNINGS

bool g_run = true;



class MyClient :public TcpClient
{
public:
	// 响应网络消息
	virtual void on_net_msg(NetDataHeader *head)
	{
		// 处理请求
		switch (head->cmd_)
		{
		case CMD_LOGIN:
		{
			NetLoginR *login = (NetLoginR*)head;
		}
		break;
		default:
			CellLog::Info("socket=<%d> receive unknown message", (int)pclient_->sockfd());
			break;
		}
	}

private:

};

int main()
{
	CellStream s;
	s.write_int8(1);
	s.write_int16(2);
	s.write_int32(3);
	s.write_float(4.0f);
	s.write_double(5.6);

	char* str = "name";
	s.write_array(str, strlen(str));
	char a[] = "passwd";
	s.write_array(a, strlen(a));
	int b[] = {1,2,3,4,5};
	s.write_array(b, 5);

	auto n1 = s.read_int8();
	auto n2 = s.read_int16();
	auto n3 = s.read_int32();
	auto n4 = s.read_float();
	auto n5 = s.read_double();
	char name[32] = {};
	auto n6 = s.read_array(name, 32);
	char pw[32] = {};
	auto n7 = s.read_array(pw, 32);
	int ata[10] = {};
	auto n8 = s.read_array(ata, 10);
	


	const char ip_linux[] = "149.28.194.79";
	const char ip_windows[] = "167.179.105.207";
	const char ip_local[] = "127.0.0.1";

	MyClient client;
	client.connect_server(ip_local, 4567);
	while (client.is_run())
	{
		client.on_run();
		CellThread::sleep(10);
	}


	return 0;
}