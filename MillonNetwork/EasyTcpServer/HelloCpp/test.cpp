#include <thread>
#include <atomic>
#include "tcp_client.hpp"
#include "cell.hpp"
#include "cell_log.hpp"
#include "cell_stream.hpp"
#include "cell_msg_stream.hpp"

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

		CellReadStream r(head);
		r.read_int16();
		r.get_cmd();
		// 处理请求
		switch (head->cmd_)
		{
		case CMD_SIGNOUT_RESULT:
		{
			
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
			int temp = 10;
		}
		break;
		default:
			CellLog::warning("socket=<%d> receive unknown message", (int)pclient_->sockfd());
			break;
		}
	}

private:

};

int main()
{
	CellLog::Instance().set_path("log.txt", "w", false);

	CellWriteStream s(128);
	s.set_cmd(CMD_SIGNOUT);
	s.write_int8(1);
	s.write_int16(2);
	s.write_int32(3);
	s.write_float(4.0f);
	s.write_double(5.6);

	s.write_string("client");
	char a[] = "passwd";
	s.write_array(a, strlen(a));
	int b[] = {1,2,3,4,5};
	s.write_array(b, 5);
	s.finish();

	const char ip_linux[] = "149.28.194.79";
	const char ip_windows[] = "167.179.105.207";
	const char ip_local[] = "127.0.0.1";

	MyClient client;
	client.connect_server(ip_local, 4567);
	
	
	while (client.is_run())
	{
		client.send_data(s.data(), s.length());
		client.on_run();
		//CellThread::sleep(10);
	}


	return 0;
}