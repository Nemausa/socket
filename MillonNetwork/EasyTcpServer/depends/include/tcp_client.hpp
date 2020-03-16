#ifndef TCPCLIENT_HPP_
#define TCPCLIENT_HPP_


#include <iostream>
using namespace std;

#include "cell.hpp"
#include "message.hpp"
#include "cell_net_work.hpp"
#include "cell_client.hpp"
#include "cell_fdset.hpp"

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
	SOCKET init_socket(int send_buffer_size=SEND_BUFF_SIZE, int recv_buffer_size=RECV_BUFF_SIZE)
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
			CELLLOG_ERROR("create socket failed...");
		}
		else
		{
			CellNetWork::make_reuseaddr(sock_);
			pclient_ = new CellClient(sock_, send_buffer_size, recv_buffer_size);
		}

		return sock_;
	}

	// 连接服务器
	int connect_server(const char* ip, unsigned short port)
	{
		if (!pclient_)
		{
			if (INVALID_SOCKET == init_socket())
			{
				return SOCKET_ERROR;
			}
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
			CellLog::error("socket=%d, ip=%s, port=%d", sock_, ip, port);
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
	bool on_run(int microseconds =1)
	{
		if (!is_run())
			return false;

		fd_read_.zero();
		fd_read_.add(sock_);
		fd_write_.zero();

		timeval tm = { 0, microseconds };
		int ret = 0;
		if(pclient_->need_write())
		{
			fd_write_.add(sock_);
			ret = select((int)sock_ + 1, fd_read_.fdset(), fd_write_.fdset(), nullptr, &tm);
		}
		else
		{
			ret = select((int)sock_ + 1, fd_read_.fdset(), nullptr, nullptr, &tm);
		}

		
		if (ret < 0)
		{
			CellLog::info("socket<%d>on_run.select exit", sock_);
			close_socket();
			return false;
		}
		if (fd_read_.has(sock_))
		{
			if (SOCKET_ERROR == recv_data(sock_))
			{
				CellLog::info("socket<%d> select task ends 2", sock_);
				close_socket();
				return false;
			}
		}

		if (fd_write_.has(sock_))
		{
			if (SOCKET_ERROR == pclient_->send_now())
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
		if (!is_run())
			return 0;

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
		if (is_run())
			return pclient_->send_data(head);
		return SOCKET_ERROR;
	}

	int send_data(const char* pdata, int len)
	{
		if (is_run())
			return pclient_->send_data(pdata, len);
		return SOCKET_ERROR;
		
	}

protected:
	CELLFDSet fd_read_;
	CELLFDSet fd_write_;
	SOCKET sock_;
	CellClient *pclient_ = nullptr;
	bool is_connect_ = false;

};



#endif
