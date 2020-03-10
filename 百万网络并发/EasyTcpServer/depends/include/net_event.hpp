#ifndef NET_EVENT_HPP_
#define NET_EVENT_HPP_

/**
* @file cell_server.hpp
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-03-06
* @author morris
* contact: tappanmorris@outlook.com
*
*/
#include "cell.hpp"
#include "cell_client.hpp"

class CellServer;

// 网络事件
class INetEvent
{
public:
	// 客户端加入事件
	virtual void on_join(CellClient* client) = 0;
	// 客户端离开事件
	virtual void on_leave(CellClient* client) = 0; // 纯虚函数  继承类必须实现函数功能
												   // 客户端消息事件
	virtual void on_net_msg(CellServer* cell_server, CellClient* client, NetDataHeader *head) = 0;

	virtual void on_recv(CellClient* client) = 0;

private:

};


#endif  // !NET_EVENT_HPP_