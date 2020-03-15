#include <iostream>
#include <vector>
#include <thread>
using namespace std;

#include "tcp_server.hpp"
#include "cell_msg_stream.hpp"
#include "cell_config.hpp"



CellTimeStamp timer;
auto t = timer.get_elapsed_second();

class MyServer :public TcpServer
{
public:
	MyServer()
	{
		sendback_ = CellConfig::Instance().exist_key("-sendback");
		sendfull_ = CellConfig::Instance().exist_key("-sendfull");
		checkid_ = CellConfig::Instance().exist_key("-checkid");
	}
	// 多线程出发 不安全
	virtual void on_net_msg(CellServer* cell_server, CellClient* pclient, NetDataHeader *head)
	{
		TcpServer::on_net_msg(cell_server, pclient, head);
		switch (head->cmd_)
		{
		case CMD_LOGIN:
		{
			pclient->reset_heart();
			NetLogin *login = (NetLogin*)head;
			// 检查消息ID
			if (checkid_)
			{
				if (login->id_ != pclient->recv_id_)
					CellLog::error("socket<%d> msgid<%d> recvid<%d> %d", pclient->sockfd(), login->id_, pclient->recv_id_, login->id_ - pclient->recv_id_);
				++pclient->recv_id_;
			}
			// 回应消息
			if (sendback_)
			{
				NetLoginR ret;
				ret.id_ = pclient->send_id_;
				if (SOCKET_ERROR == pclient->send_data(&ret))
				{
					// 发送缓冲区满了，消息还没有发出去，目前直接抛弃
					// 客户端消息太多了，需要考虑对应的策略
					// 正常连接，业务端不会有那么多消息
					// 模拟并发测试时是否发送频率过高
					if (sendfull_)
					{
						CellLog::warning("<socket=%d> send full", pclient->sockfd());
					}
				}
				else
				{
					++pclient->send_id_;
				}
			}
			
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
			pclient->send_data(s.data(), s.length());
			
		}
		break;
		case CMD_HEART_C2S:
		{
			pclient->reset_heart();
			Net_C2S_Heart ret;
			pclient->send_data(&ret);
		}
		break;
		default:
			NetDataHeader ret = {};
			//send_data(csock, &ret);
			CellLog::error("command CMD_ERROR socket=<%d> data length=<%d>", (int)pclient->sockfd(), ret.length_);
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
	bool sendback_ = true;
	bool sendfull_;
	bool checkid_;
};



int main(int argc, char* args[])
{
	CellLog::Instance().set_path("server_log", "w", false);

	CellConfig::Instance().init(argc, args);
	const char* ip = CellConfig::Instance().get_str("ip", "any");
	uint16_t port = CellConfig::Instance().get_int("port", 4567);
	int n_thread = CellConfig::Instance().get_int("thread", 1);

	if (CellConfig::Instance().exist_key("-p"))
	{
		CellLog::info("exist key");
	}

	if (strcmp(ip, "any") == 0)
		ip = nullptr;

	
	
	MyServer server1;
	server1.init_socket();
	server1.bind_port(ip, port);
	server1.listen_port(64);
	server1.start(n_thread);


	while (true)
	{
		char buffer[256] = {};
		scanf("%s", buffer);
		if (0 == strcmp(buffer, "exit"))
		{
			CellLog::info("退出线程");
			server1.close_socket();
			break;;
		}
		else
		{
			CellLog::warning("不支持的命令");
		}
		std::chrono::microseconds dura(1);
		std::this_thread::sleep_for(dura);
	}
	

	CellLog::info("已退出");
	return 0;
}

