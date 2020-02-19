#define WIN32_LEAN_AND_MEAN  // 避免早期定义的宏冲突
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
		// 3.输入请求命令
		char cmdbuf[128] = {};
		scanf_s("%s", cmdbuf, 128);
		// 4.处理请求
		if (0 == strcmp(cmdbuf, "exit"))
		{
			cout << "c程序退出" << endl;
			break;
		}
		else if (0 == strcmp(cmdbuf, "login"))
		{
			Login login;
			strcpy_s(login.username_, "Kevin");
			strcpy_s(login.passwd_, "passwd");

			// 向服务器发送请求命令
			send(_sock, (const char*)&login, sizeof(login), 0);
			LoginResult ret_login = {};
			recv(_sock, (char*)&ret_login, sizeof(LoginResult), 0);
			cout << "loginresut:" << ret_login.result_ << endl;
		}
		else if (0 == strcmp(cmdbuf, "signout"))
		{
			SignOut signout;
			strcpy_s(signout.username_, "Kevin");
			// 向服务器发送命令
			send(_sock, (const char*)&signout, sizeof(signout), 0);
			// 接受服务器返回的数据
			SignOutResult ret_sign = {};
			recv(_sock, (char*)&ret_sign, sizeof(ret_sign), 0);
			cout << "signout: " << ret_sign.result_ << endl;
		}
		else
		{
			cout << "不支持的命令" << endl;
		}

		
		
	}

	

	// 4.关闭套接字
	closesocket(_sock);
	WSACleanup();

	getchar();
	return 0;
}