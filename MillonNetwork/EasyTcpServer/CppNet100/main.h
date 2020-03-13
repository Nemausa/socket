
#ifndef _CPP_NET_100_DLL_H_
#define _CPP_NET_100_DLL_H_

#ifdef _WIN32
#define EXPORT_DLL _declspec(dllexport)
#else
#define EXPORT_DLL
#endif


#include <string>
#include "tcp_client.hpp"



extern "C" // 请按照C的方式导出
{
	typedef void(*msg_callback)(void* creator, void* data, int len);
}



class NativeTcpClient :public TcpClient
{
public:
	// 响应网络消息
	virtual void on_net_msg(NetDataHeader *head)
	{
		if (callback_)
			callback_(creator_, head, head->length_);
	}

	void set_callback(void* creator, msg_callback cb)
	{
		creator_ = creator;
		callback_ = cb;
	}
private:
	void* creator_ = nullptr;
	msg_callback callback_ = nullptr;
};



extern "C" // 请按照C的方式导出
{
	 _declspec(dllexport)/*导出C++函数*/ int add(int a, int b)
	{
		return a + b;
	}

	typedef void(*callback1)(const char* str);

	_declspec(dllexport) void TestCall(const char* str1, callback1 cb)
	{
		std::string s = "hello ";
		s += str1;
		cb(s.c_str());
	}
	//////////////////////////////////////////////////////////////////////////

	EXPORT_DLL  void* cell_client_create(void* creator, msg_callback cb)
	{
		NativeTcpClient* pclient = new NativeTcpClient();
		pclient->set_callback(creator, cb);
		return pclient;
	}

	EXPORT_DLL bool cell_client_connect(NativeTcpClient* pclient, const char* ip, short port)
	{
		if(pclient && ip)
			return SOCKET_ERROR != pclient->connect_server(ip, port);
		return false;
	}

	EXPORT_DLL bool  cell_client_onrun(NativeTcpClient* pclient)
	{
		if (pclient)
			return pclient->on_run();
		return false;
	}

	EXPORT_DLL void  cell_client_close(NativeTcpClient* pclient)
	{
		if (pclient)
		{
			pclient->close_socket();
			if(pclient)
				delete pclient;
		}
			
	}

	EXPORT_DLL int  cell_client_send(NativeTcpClient* pclient, const char* data, int len)
	{
		if (!pclient)
			return 0;
		return pclient->send_data(data, len);
	}
}






#endif   // !_CPP_NET_100_DLL_H_