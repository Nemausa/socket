#ifndef TCPCLIENT_HPP_
#define TCPCLIENT_HPP_

#include "cell.hpp"
#include "message.hpp"
#include <iostream>
using namespace std;
//缓冲区最小单元的大小
#define  RECV_BUFF_SIZE 10240
class TcpClient
{
public:
	SOCKET sock_;
	bool is_connect_;
	//char sz_recv_buf_[RECV_BUFF_SIZE];		// 接收缓冲区
	char sz_msg_buf_[RECV_BUFF_SIZE];	// 消息缓冲区 
	int last_pos_;
public:
	TcpClient()
	{
		sock_ = INVALID_SOCKET;
		last_pos_ = 0;
		is_connect_ = false;
		//memset(sz_recv_buf_, 0, RECV_BUFF_SIZE);
		memset(sz_msg_buf_, 0, RECV_BUFF_SIZE);

	}

	virtual ~TcpClient()
	{
		close_socket();
		
	}

	// 初始化socket
	void init_socket()
	{
		// 启动win socke 2.x环境
#ifdef _WIN32
		// 启动socket 网络环境
		WORD version = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(version, &dat);
#endif
		// 建立一个TCP客户端
		// 1.建立一个socket
		if (INVALID_SOCKET != sock_)
		{
			cout << "close old socket" << endl;
			close_socket();
		}
		sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		
		if (INVALID_SOCKET == sock_)
		{
			cout << "socket error" << endl;
		}
		/*else
			cout << "socket success" << endl;*/
	}

	// 连接服务器
	int connect_server(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == sock_)
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
			cout << "socket=" << sock_ <<"ip" << ip <<"port="<< port << "connect error" << endl;
		}
		else
		{
			is_connect_ = true;
		}
			//cout << "socket=" << sock_ <<"ip" << ip << "port=" << port << "connect success" << endl;*/

		return ret;
	}

	// 关闭socket
	void close_socket()
	{
		
		if (INVALID_SOCKET == sock_)
			return;
		
#ifdef _WIN32
		// 4.关闭套接字
		closesocket(sock_);
		WSACleanup();
#else
		close(sock_);
#endif
		sock_ = INVALID_SOCKET;
		is_connect_ = false;
	}


	// 处理网络消息
	bool on_run()
	{
		if (!is_run())
			return false;

		fd_set fd_read;
		FD_ZERO(&fd_read);
		FD_SET(sock_, &fd_read);

		timeval tm = { 0, 1 };
		int ret = select((int)sock_+1, &fd_read, 0, 0, &tm);
		if (ret < 0)
		{
			cout << "select taks ends 1" << endl;
			close_socket();
			return false;
		}
		if (FD_ISSET(sock_, &fd_read))
		{
			FD_CLR(sock_, &fd_read);
			if (-1 == recv_data(sock_))
			{
				cout << "select task ends 2" << endl;
				close_socket();
				return false;
			}
		}

		return true;
	}

	// 是否运行
	bool is_run()
	{
		return INVALID_SOCKET != sock_;
	}

	
	// 接受数据 处理粘包 拆包
	int recv_data(SOCKET _csock)
	{
		int len_head = sizeof(NetDataHeader);
		
		char *sz_recv_buf = sz_msg_buf_ + last_pos_;
		// 接受数据
		int len = recv(sock_, sz_recv_buf, RECV_BUFF_SIZE-last_pos_, 0);
		if (len <= 0)
		{
			printf("socket=<%d> disconnect from server", (int)sock_);
			return -1;
		}

		// 将收取的数据拷贝到消息缓冲区
		//memcpy(sz_msg_buf_+last_pos_, sz_recv_buf_, len);
		// 消息缓冲区的数据尾部位置后移
		last_pos_ += len;
		// 判断消息缓冲区的数据长度大于消息头的长度
		while (last_pos_ >= sizeof(NetDataHeader))
		{
			// 这时就可以知道当前消息体的长度
			NetDataHeader *head = (NetDataHeader*)sz_msg_buf_;
			// 判断消息缓冲区的数据长度大于消息长度
			if (last_pos_ >= head->length_)
			{
				// 剩余未处理消息缓冲区的长度
				int size = last_pos_ - head->length_;
				// 处理网络消息
				on_net_msg(head);
				// 将剩余未处理消息缓冲前移
				memcpy(sz_msg_buf_, sz_msg_buf_ + head->length_, size);
				last_pos_ = size;	
			}
			else
			{
				// 剩余缓冲区数据不够一条完整的消息
				break;
			}

		}

		return 0;
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
			//cout << "socket=" << sock_ << "command:CMD_LOGIN_RESULT" << "data length:" << login->length_ << endl;
		}
		break;
		case CMD_SIGNOUT_RESULT:
		{
			NetLoginR *loginout = (NetLoginR*)head;
			//cout << "socket=" << sock_ << "command:CMD_SIGNOUT_RESULT" << "data length:" << loginout->length_ << endl;
		}
		break;
		case  CMD_NEW_USER_JOIN:
		{
			NetNewUserJoin *user = (NetNewUserJoin*)head;
			//cout << "socket=" << sock_ << "command:CMD_NEW_USER_JOIN" << "data length:" << user->length_ << endl;
		}
		break;
		case  CMD_ERROR:
		{
			printf("socket=<%d> receive error, data length=<%d>\n", (int)sock_, head->length_);
		}
		break;
		default:
			printf("socket=<%d> receive unknown message, data length=<%d>\n", (int)sock_, head->length_);
			break;
		}
	}

	// 发送数据
	int send_data(NetDataHeader *head, int length)
	{
		int ret = SOCKET_ERROR;
		if (is_run() && head)
		{
			ret = send(sock_, (const char*)head, length, 0);
			if (SOCKET_ERROR == ret)
			{
				close_socket();
			}
		}
		return ret;
			
	}

private:

};



#endif
