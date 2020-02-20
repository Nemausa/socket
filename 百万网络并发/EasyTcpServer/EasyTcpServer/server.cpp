#define WIN32_LEAN_AND_MEAN  // 避免早期定义的宏冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#include <vector>
using namespace std;
#pragma comment(lib,"ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_SIGNOUT,
	CMD_SIGNOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};

struct DataHeader
{
	short cmd_;
	short length_;
};

struct Login: public DataHeader
{
	Login(){cmd_ = CMD_LOGIN; length_ = sizeof(Login);}
	char username_[32];
	char passwd_[32];
};

struct LoginResult:public DataHeader
{
	LoginResult() { cmd_ = CMD_LOGIN_RESULT, length_ = sizeof(LoginResult); result_ = 0; }
	int result_;
};

struct SignOut:public DataHeader
{
	SignOut() { cmd_ = CMD_SIGNOUT, length_ = sizeof(SignOut); }
	char username_[32];
};

struct SignOutResult:public DataHeader
{
	SignOutResult() { cmd_ = CMD_SIGNOUT_RESULT, length_ = sizeof(SignOutResult); result_ = 0; }
	int result_;
};

struct NewUserJoin :public DataHeader
{
	NewUserJoin() { cmd_ = CMD_NEW_USER_JOIN, length_ = sizeof(NewUserJoin); id_socket = 0; }
	int id_socket;
};


vector<SOCKET> g_clients;

int process(SOCKET _csock);

int main()
{
	// 启动socket 网络环境
	WORD version = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(version, &dat);

	// 建立一个TCP服务器
	// 1.建立一个socket
	SOCKET _sock = socket(AF_INET/*IPV4*/, SOCK_STREAM/*数据流*/, IPPROTO_TCP);
	// 2.bind 绑定用于接受客户端的网络端口
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1");
	int bs = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
	if (SOCKET_ERROR == bs)
	{
		cout << "Error 绑定错误" << endl;
	}
	// 3.监听网络端口
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		cout << "Error 监听失败" << endl;
	}

	

	
	while (true)
	{
		// 伯克利 socket
		//select(
		//   _In_ int nfds, windows无意义
		//	_Inout_opt_ fd_set FAR * readfds,		读集合
		//	_Inout_opt_ fd_set FAR * writefds,		写集合
		//	_Inout_opt_ fd_set FAR * exceptfds,		错误集合
		//	_In_opt_ const struct timeval FAR * timeout   空则阻塞下去
		//	);
		fd_set fd_read;
		fd_set fd_write;
		fd_set fd_except;
		
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_except);

		FD_SET(_sock, &fd_read);
		FD_SET(_sock, &fd_write);
		FD_SET(_sock, &fd_except);

		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			FD_SET(g_clients[n], &fd_read);
		}

		// nfds 是一个整数值，是指fd_set集合所有的描述符(socket)的范围，而不是数量
		// 既是所有文件描述符最大值+1，在windows中这个参数可以写0
		timeval tm = {1, 0};
		int ret = select(_sock+1, &fd_read, &fd_write, &fd_except, &tm);
		if (ret < 0)
		{
			cout << "select任务结束" << endl;
			break;
		}
		if (FD_ISSET(_sock, &fd_read))  // 判断集合是否有可操作的socket
		{
			FD_CLR(_sock, &fd_read);
			// 4.等待客户端连接
			sockaddr_in client_addr = {};
			SOCKET _csock = INVALID_SOCKET;
			int n_addr = sizeof(sockaddr_in);

			char buffer[] = "Hello, I'm Server.";

			_csock = accept(_sock, (sockaddr*)&client_addr, &n_addr);
			if (INVALID_SOCKET == _sock)
			{
				cout << "Error 无效的客户端socket" << endl;
			}
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				NewUserJoin user = {};
				send(g_clients[n], (const char*)&user, sizeof(NewUserJoin), 0);
			}

			cout << "新客户端socket " << _csock << "IP: " << inet_ntoa(client_addr.sin_addr) << endl;
			g_clients.push_back(_csock);
		}

		for (size_t n = 0; n < fd_read.fd_count; n++)
		{
			if (-1 == process(fd_read.fd_array[n]))
			{
				auto iter = find(g_clients.begin(), g_clients.end(), fd_read.fd_array[n]);
				if (iter != g_clients.end())
					g_clients.erase(iter);
			}
		}

		cout << "空闲时间处理其他业务" << endl;

	}
	

	for (int n = (int)g_clients.size() - 1; n >= 0; n--)
	{
		closesocket(g_clients[n]);
	}
	// 关闭套接字
	closesocket(_sock);
	WSACleanup();
	return 0;
}

int process(SOCKET _csock)
{
	int len_head = sizeof(DataHeader);
	// 缓冲区
	char recv_buf[1024] = {};
	// 接受客户端的请求
	int len = recv(_csock, recv_buf, sizeof(DataHeader), 0);
	DataHeader *head = (DataHeader*)recv_buf;
	if (len <= 0)
	{
		cout << "客户端已经退出，任务结束" << endl;
		return -1;
	}
	// 处理请求
	switch (head->cmd_)
	{
	case CMD_LOGIN:
	{
		recv(_csock, recv_buf + len_head, head->length_ - len_head, 0);
		Login *login = (Login*)recv_buf;
		cout << "收到命令" << head->cmd_ << "socket:" << _csock << "数据长度:" << login->length_ << " userName:" << login->username_ << " passwd:" << login->passwd_ << endl;
		// 判断用户密码正确的过程
		LoginResult ret;
		send(_csock, (char*)&ret, sizeof(LoginResult), 0);
	}
	break;
	case CMD_SIGNOUT:
	{
		recv(_csock, recv_buf + len_head, head->length_ - len_head, 0);
		SignOut *loginout = (SignOut*)recv_buf;
		cout << "收到命令" << head->cmd_ << "socket:" << _csock << "数据长度:" << loginout->length_ << " userName:" << loginout->username_ << endl;
		// 判断用户密码正确的过程
		SignOutResult ret = {};
		send(_csock, (char*)&ret, sizeof(SignOutResult), 0);
	}
	break;
	default:
		DataHeader head = { CMD_ERROR, 0 };
		send(_csock, (char*)&head, sizeof(DataHeader), 0);
		break;
	}

	return 0;
}