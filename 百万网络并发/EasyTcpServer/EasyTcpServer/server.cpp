#include <iostream>
#include <vector>
using namespace std;
#include "tcp_server.hpp"
#include <thread>

bool g_run = true;
void cmd()
{
	while (true)
	{
		char buffer[256] = {};
		scanf("%s", buffer);
		if (0 == strcmp(buffer, "exit"))
		{
			g_run = false;
			cout << "退出线程" << endl;
			return;
		}
		else
		{
			cout << "不支持的命令" << endl;
		}
	}

}

int main()
{
	TcpServer server1;
	server1.init_socket();
	server1.bind_port(nullptr, 4567);
	server1.listen_port(5);
	server1.start();

	thread cmd_thread1(cmd);
	cmd_thread1.detach();

	while (g_run)
	{	
		server1.on_run();
	}
	server1.close_socket();
	printf("已退出\n");
	return 0;
}

