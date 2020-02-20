﻿#define WIN32_LEAN_AND_MEAN  // 避免早期定义的宏冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#include <stdio.h>
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

struct Login : public DataHeader
{
	Login() { cmd_ = CMD_LOGIN; length_ = sizeof(Login); }
	char username_[32];
	char passwd_[32];
};

struct LoginResult :public DataHeader
{
	LoginResult() { cmd_ = CMD_LOGIN_RESULT, length_ = sizeof(LoginResult); result_ = 0; }
	int result_;
};

struct SignOut :public DataHeader
{
	SignOut() { cmd_ = CMD_SIGNOUT, length_ = sizeof(SignOut); }
	char username_[32];
};

struct SignOutResult :public DataHeader
{
	SignOutResult() { cmd_ = CMD_SIGNOUT_RESULT, length_ = sizeof(SignOutResult); result_ = 0; }
	int result_;
};

struct NewUserJoin :public DataHeader
{
	NewUserJoin() { cmd_ = CMD_NEW_USER_JOIN, length_ = sizeof(NewUserJoin); id_socket = 0; }
	int id_socket;
};
int process(SOCKET _csock);

int main()
{
	// 启动socket 网络环境
	WORD version = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(version, &dat);

	// 建立一个TCP客户端
	// 1.建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		cout << "socket error" << endl;
	}
	else
		cout << "socket success" << endl;
	// 2.连接服务器
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (SOCKET_ERROR == connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr)))
	{
		cout << "connect error" << endl;
	}
	else
		cout << "connect success" << endl;

	
	while (true)
	{
		fd_set fd_read;
		FD_ZERO(&fd_read);
		FD_SET(_sock, &fd_read);

		timeval tm = {1, 0};
		int ret = select(_sock, &fd_read, 0, 0, &tm);
		if (ret < 0)
		{
			cout << "select任务结束1" << endl;
		}
		if (FD_ISSET(_sock, &fd_read))
		{
			FD_CLR(_sock, &fd_read);
			if (-1 == process(_sock))
			{
				cout << "select任务结束2" << endl;
			}
		}
		
		Login login;
		strcpy_s(login.username_, "Kevin");
		strcpy_s(login.passwd_, "passwd");

		cout << "空闲时间处理其他业务" << endl;
		send(_sock, (const char*)&login, sizeof(Login), 0);
		Sleep(1000);
	}

	cout << "已经退出" << endl;

	// 4.关闭套接字
	closesocket(_sock);
	WSACleanup();
	getchar();
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
		cout << "与服务器端口连接，任务结束" << endl;
		return -1;
	}
	// 处理请求
	switch (head->cmd_)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(_csock, recv_buf + len_head, head->length_ - len_head, 0);
		LoginResult *login = (LoginResult*)recv_buf;
		cout << "收到命令:CMD_LOGIN_RESULT" << head->cmd_ << "socket:" << _csock << "数据长度:" << login->length_ << endl;
	}
	break;
	case CMD_SIGNOUT_RESULT:
	{
		recv(_csock, recv_buf + len_head, head->length_ - len_head, 0);
		SignOutResult *loginout = (SignOutResult*)recv_buf;
		cout << "收到命令:CMD_SIGNOUT_RESULT" << head->cmd_ << "socket:" << _csock << "数据长度:" << loginout->length_ << endl;
	}
	break;
	case  CMD_NEW_USER_JOIN:
	{
		recv(_csock, recv_buf + len_head, head->length_ - len_head, 0);
		NewUserJoin *user = (NewUserJoin*)recv_buf;
		cout << "收到命令:CMD_NEW_USER_JOIN"<< "socket:" << _csock << "数据长度:" << user->length_ << endl;
	} 
	break;
	default:
		DataHeader head = { CMD_ERROR, 0 };
		send(_csock, (char*)&head, sizeof(DataHeader), 0);
		break;
	}

	return 0;
}