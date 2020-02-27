#include "tcp_client.hpp"


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

const int t_count = 4;    // 线程数量
const int c_count = 1000; // 客户端数量
TcpClient *client[c_count];

void send_thread(int id)
{
	int c = c_count / t_count;
	int begin = (id - 1)*c;
	int end = id*c;
	// linux	149.28.194.79
	// windows	167.179.105.207
	const char ip_linux[] = "149.28.194.79";
	const char ip_windows[] = "167.179.105.207";
	const char ip_local[] = "127.0.0.1";
	//const char ip_local[] = "192.168.1.101";

	for (int n = begin; n < end; n++)
	{
		client[n] = new TcpClient();
	}
	for (int n = begin; n < end; n++)
	{
		client[n]->init_socket();
		client[n]->connect_server(ip_linux, 4567);
		printf("connect<%d>\n", n + 1);
	}

	std::chrono::milliseconds t(5000);
	std::this_thread::sleep_for(t);

	Login login[10];
	for (int n = 0; n < 10; n++)
	{
		strcpy(login[n].passwd_, "passwd");
		strcpy(login[n].username_, "Morris");
	}

	int len = sizeof(login);

	while (g_run)
	{
		for (int n = begin; n < end; n++)
		{
			client[n]->send_data(login, len);
		}
	}

	for (int n = begin; n < end; n++)
		client[n]->close_socket();
}

int main()
{
	

	// UI线程
	thread cmd_thread(cmd);
	cmd_thread.detach();

	
	// 启动发送线程
	for (int n = 0; n < t_count; n++)
	{
		thread t1(send_thread, n+1);
		t1.detach();
	}
	
	while (g_run)
		Sleep(100);

	cout << "exited" << endl;

	
	return 0;
}

