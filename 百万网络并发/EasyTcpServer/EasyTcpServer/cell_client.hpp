#ifndef CELL_CLIENT_HPP_
#define CELL_CLIENT_HPP_

/**
* @file cell_client.hpp
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


// 客户端数据类型
class CellClient
{
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET)
	{
		sockfd_ = sockfd;
		last_msg_pos_ = 0;
		last_send_pos_ = 0;
		memset(sz_msg_buf_, 0, RECV_BUFF_SIZE);
		memset(sz_send_buf_, 0, SEND_BUFF_SIZE);
	}
	virtual ~CellClient()
	{

	}

	SOCKET sockfd()
	{
		return sockfd_;
	}

	char *msg_buf()
	{
		return sz_msg_buf_;
	}

	int get_last_pos()
	{
		return last_msg_pos_;
	}

	void set_last_pos(int pos)
	{
		last_msg_pos_ = pos;
	}

	// 发送数据给指定的客户端
	int send_data(NetDataHeader *head)
	{
		int ret = SOCKET_ERROR;
		// 发送的数据长度
		int send_len = head->length_;
		// 要发送的数据
		const char* pSendData = (const char*)head;
		while (true)
		{
			if (last_send_pos_ + send_len >= SEND_BUFF_SIZE)
			{
				// 计算可以拷贝的数据长度
				int copy_len = SEND_BUFF_SIZE - last_send_pos_;
				// 拷贝数据到发送缓冲区
				memcpy(sz_send_buf_ + last_send_pos_, pSendData, copy_len);
				// 计算剩余数据位置
				pSendData += copy_len;
				// 剩余数据长度
				send_len -= copy_len;

				ret = send(sockfd_, sz_send_buf_, SEND_BUFF_SIZE, 0);
				// 数据尾部位置清零
				last_send_pos_ = 0;
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else
			{
				// 将要发送的数据拷贝到发送缓冲区尾部
				memcpy(sz_send_buf_ + last_send_pos_, pSendData, send_len);
				// 数据尾部位置
				last_send_pos_ += send_len;
				break;
			}
		}


		return ret;
	}


private:
	SOCKET sockfd_; // socket fd_set file desc set
	char sz_msg_buf_[RECV_BUFF_SIZE];  // 第二缓冲区 消息缓冲区
	char sz_send_buf_[SEND_BUFF_SIZE];  // 第二缓冲区 发送缓冲区
	int last_msg_pos_;  // 消息缓冲区的数据尾部位置
	int last_send_pos_; // 发送缓冲区尾部位置
};

#endif  // !CELL_CLIENT_HPP_