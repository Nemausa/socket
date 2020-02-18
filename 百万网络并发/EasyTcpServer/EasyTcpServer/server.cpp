#define WIN32_LEAN_AND_MEAN  // 避免早期定义的宏冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <iostream>
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
	cout << "新客户端socket " << _csock << "IP:" << inet_ntoa(client_addr.sin_addr) << endl;

	char recvbuf[128] = {};
	while (true)
	{
		// 接受客户端的请求
		int len = recv(_csock, recvbuf, 128, 0);
		if (len <= 0)
		{
			cout << "客户端已经退出，任务结束" << endl;
			break;
		}
		// 处理请求
		if (0 == strcmp(recvbuf,"getinfo"))
		{
			DataPackage dp = {25, "Kevin"};
			send(_csock,(const char*)&dp, sizeof(DataPackage), 0);
		}
		else
		{
			char msgbuf[] = "???";
			send(_csock, msgbuf, strlen(msgbuf) + 1, 0);
		}

		// 5.发送消息
		//cout <<"新客户端IP:" << inet_ntoa(client_addr.sin_addr)<< endl;
		//send(_csock, buffer, strlen(buffer) + 1, 0);
	}
	

	// 6.关闭套接字
	closesocket(_sock);
	WSACleanup();
	return 0;
}