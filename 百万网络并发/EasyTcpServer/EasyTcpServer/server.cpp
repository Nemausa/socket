#include <iostream>
#include <vector>
using namespace std;
#include "tcp_server.hpp"


int main()
{
	TcpServer server1;
	server1.init_socket();
	server1.bind_port(nullptr, 4567);
	server1.listen_port(5);



	while (server1.is_run())
	{	
		server1.on_run();
	}
	server1.close_socket();
	printf("已退出\n");
	return 0;
}

