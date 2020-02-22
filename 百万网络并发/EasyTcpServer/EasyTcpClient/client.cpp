#include "tcp_client.hpp"



int process(SOCKET _csock);

bool g_run = true;

void cmd(TcpClient *client)
{
	while (true)
	{
		char buffer[256] = {};
		scanf("%s", buffer);
		if (0 == strcmp(buffer, "exit"))
		{
			client->close_socket();
			cout << "退出线程" << endl;
			return;
		}
		else if (0 == strcmp(buffer, "login"))
		{
			Login login;
			strcpy(login.username_, "Kevin");
			strcpy(login.passwd_, "passwd");
			client->send_data(&login);
		}
		else if (0 == strcmp(buffer, "signout"))
		{
			SignOut signout;
			strcpy(signout.username_, "Kevin");
			client->send_data(&signout);
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

	TcpClient client1;
	client1.init_socket();
	client1.connect_server(ip_windows, 4567);
	 
	//TcpClient client2;
	//client2.init_socket();
	//client2.connect_server("127.0.0.1", 4567);


	thread cmd_thread1(cmd, &client1);
	cmd_thread1.detach();

	//thread cmd_thread2(cmd, &client2);
	//cmd_thread2.detach();
	
	Login login;
	strcpy(login.username_, "Kevin");
	strcpy(login.passwd_, "passwd");

	while (client1.is_run())
	{
		client1.on_run();
		client1.send_data(&login);
	}

	client1.close_socket();
	//client2.close_socket();
	cout << "exited" << endl;

	getchar();
	return 0;
}

