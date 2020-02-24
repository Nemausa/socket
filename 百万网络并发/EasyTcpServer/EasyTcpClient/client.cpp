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

int main()
{
	// linux	149.28.194.79
	// windows	167.179.105.207
	const char ip_linux[] = "149.28.194.79";
	const char ip_windows[] = "167.179.105.207";
	const char ip_local[] = "127.0.0.1";
	//const char ip_local[] = "192.168.1.101";

	const int size = FD_SETSIZE - 1;
	//const int size = 100;
	TcpClient *client[size];
	int a = sizeof(client);

	for (int n = 0; n < size; n++)
	{
		client[n] = new TcpClient();
	}
	for (int n = 0; n < size; n++)
	{
		client[n]->init_socket();
		client[n]->connect_server(ip_local, 4567);
	}
	
	 
	//TcpClient client2;
	//client2.init_socket();
	//client2.connect_server("127.0.0.1", 4567);


	thread cmd_thread1(cmd);
	cmd_thread1.detach();
	
	

	//thread cmd_thread2(cmd, &client2);
	//cmd_thread2.detach();
	
	Login login;
	strcpy(login.username_, "Kevin");
	strcpy(login.passwd_, "passwd");

	while (g_run)
	{

		for (int n = 0; n < size; n++)
		{
			//client[n]->on_run();
			client[n]->send_data(&login);
		}
	}

	for (int n = 0; n < size; n++)
	client[n]->close_socket();
	//client2.close_socket();
	cout << "exited" << endl;

	getchar();
	return 0;
}

