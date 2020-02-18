#define WIN32_LEAN_AND_MEAN  // 避免早期定义的宏冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#include <stdio.h>
using namespace std;

#pragma comment(lib,"ws2_32.lib")


struct DataPackage
{
	int age;
	char name[32];
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
		else
			// 5.向服务器发送请求命令
			send(_sock, cmdbuf, strlen(cmdbuf)+1, 0);

		// 3.接受服务器发送的信息
		char recvbuf[256] = {};
		int nlen = recv(_sock, recvbuf, 256, 0);
		if (nlen > 0)
		{
			DataPackage *dp = (DataPackage*)recvbuf;
			cout << "接收到数据:" << dp->age << ":" << dp->name  << endl;
		}
	}

	

	// 4.关闭套接字
	closesocket(_sock);
	WSACleanup();

	getchar();
	return 0;
}