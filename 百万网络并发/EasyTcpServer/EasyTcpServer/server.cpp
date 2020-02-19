#define WIN32_LEAN_AND_MEAN  // 避免早期定义的宏冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <iostream>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

enum CMD
{
	CMD_LOGIN,
	CMD_SIGNOUT,
	CMD_ERROR
};

struct DataHeader
{
	short cmd_;
	short length_;
};

struct Login
{
	char username_[32];
	char passwd_[32];
};

struct LoginResult
{
	int result_;
};

struct SignOut
{
	char username_[32];
};

struct SignOutResult
{
	int result_;
};


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
	cout << "新客户端socket " << _csock << "IP: " << inet_ntoa(client_addr.sin_addr) << endl;

	
	while (true)
	{
		DataHeader head = {};
		// 接受客户端的请求
		int len = recv(_csock, (char*)&head, sizeof(head), 0);
		if (len <= 0)
		{
			cout << "客户端已经退出，任务结束" << endl;
			break;
		}
		cout << "受到命令：" << head.cmd_ << "数据长度:" << head.length_ << endl;
		// 处理请求
		switch (head.cmd_)
		{
		case CMD_LOGIN:
		{
			Login login = {};
			recv(_csock, (char*)&login, sizeof(Login), 0);
			// 判断用户密码正确的过程
			LoginResult ret = {1};
			send(_csock, (char*)&head, sizeof(DataHeader), 0);
			send(_csock, (char*)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_SIGNOUT:
		{
			SignOut loginout = {1};
			recv(_csock, (char*)&loginout, sizeof(SignOut), 0);
			// 判断用户密码正确的过程
			SignOutResult ret = {};
			send(_csock, (char*)&head, sizeof(DataHeader), 0);
			send(_csock, (char*)&ret, sizeof(SignOutResult), 0);
		}
		break;
		default:
			head.cmd_ = CMD_ERROR;
			head.length_ = 0;
			send(_csock, (char*)&head, sizeof(DataHeader), 0);
			break;
		}

	}
	

	// 6.关闭套接字
	closesocket(_sock);
	WSACleanup();
	return 0;
}