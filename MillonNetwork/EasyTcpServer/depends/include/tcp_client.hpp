#ifndef TCPCLIENT_HPP_
#define TCPCLIENT_HPP_


#include <iostream>
using namespace std;

#include "cell.hpp"
#include "message.hpp"
#include "cell_net_work.hpp"
#include "cell_client.hpp"

//缓冲区最小单元的大小
#define  RECV_BUFF_SIZE 10240
class TcpClient
{
public:
	TcpClient()
	{
		is_connect_ = false;
	}

	virtual ~TcpClient()
	{
		close_socket();
	}

	// 初始化socket
	void init_socket()
	{
		CellNetWork::Init();
		// 建立一个TCP客户端
		// 1.建立一个socket
		if (pclient_)
		{
			CellLog::info("<socket<%d> close old socket...\n", sock_);
			close_socket();
		}
		sock_ = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
		
		if (INVALID_SOCKET == sock_)
		{
			cout << "socket error" << endl;
		}
		else
		{
			pclient_ = new CellClient(sock_);
		}
	}

	// 连接服务器
	int connect_server(const char* ip, unsigned short port)
	{
		if (!pclient_)
		{
			init_socket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(sock_, (sockaddr*)&_sin, sizeof(sockaddr));
		if (SOCKET_ERROR == ret)
		{
			CellLog::info("socket=%d, ip=%s, port=%d", sock_, ip, port);
		}
		else
		{
			is_connect_ = true;
		}
		return ret;
	}

	// 关闭socket
	void close_socket()
	{
		
		if (pclient_)
		{
			delete pclient_;
			pclient_ = nullptr;
		}
		is_connect_ = false;
	}


	// 处理网络消息
	bool on_run()
	{
		if (!is_run())
			return false;

		fd_set fd_read;
		FD_ZERO(&fd_read);
		fd_set fd_write;
		FD_ZERO(&fd_write);
		
		timeval tm = { 0, 1 };
		int ret = 0;
		FD_SET(sock_, &fd_read);
		if(pclient_->need_write())
		{
			FD_SET(sock_, &fd_write);
			ret = select((int)sock_ + 1, &fd_read, &fd_write, 0, &tm);
		}
		else
		{
			ret = select((int)sock_ + 1, &fd_read, nullptr, 0, &tm);
		}
			

		
		
		if (ret < 0)
		{
			CellLog::info("socket<%d>on_run.select error", sock_);
			close_socket();
			return false;
		}
		if (FD_ISSET(sock_, &fd_read))
		{
			if (-1 == recv_data(sock_))
			{
				CellLog::info("socket<%d> select task ends 2", sock_);
				close_socket();
				return false;
			}
		}

		if (FD_ISSET(sock_, &fd_write))
		{
			if (-1 == pclient_->send_now())
			{
				CellLog::info("socket<%d> select task ends 2", sock_);
				close_socket();
				return false;
			}
		}

		return true;
	}

	// 是否运行
	bool is_run()
	{
		return pclient_ && is_connect_;
	}

	
	// 接受数据 处理粘包 拆包
	int recv_data(SOCKET _csock)
	{
		int len = pclient_->recv_data();
		if (len > 0)
		{
			while (pclient_->has_msg())
			{
				on_net_msg(pclient_->front_msg());
				pclient_->pop();
			}
		}
		return len;
	}

	virtual void on_net_msg(NetDataHeader *head) = 0;

	// 发送数据
	int send_data(NetDataHeader *head)
	{
		return pclient_->send_data(head);	
	}

	int send_data(const char* pdata, int len)
	{
		return pclient_->send_data(pdata, len);
	}

protected:
	CellClient *pclient_ = nullptr;
	SOCKET sock_;
	bool is_connect_;

};



#endif
