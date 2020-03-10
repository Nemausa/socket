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
#include "cell_buffer.hpp"

// 客户端心跳检测死亡计时
#define CLIENT_HEART_DEAD_TIME 60000
// 指定时间发送缓冲区数据
#define CLIENT_SEND_TIME 200

// 客户端数据类型
class CellClient
{
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET):send_buf_(SEND_BUFF_SIZE),recv_buf_(RECV_BUFF_SIZE)
	{
		static int n = 1;
		id = n++;
		sockfd_ = sockfd;
		reset_send();
		reset_heart();
	}
	virtual ~CellClient()
	{
		CellLog::Info("server=%d, CellClient%d.~CellClient\n", server_id,id);
		if (INVALID_SOCKET == sockfd_)
			return;

#ifdef _WIN32
		closesocket(sockfd_);
#else
		close(sockfd_);
#endif
	}

	SOCKET sockfd()
	{
		return sockfd_;
	}

	int recv_data()
	{
		return recv_buf_.read_socket(sockfd_);
	}

	bool has_msg()
	{
		return recv_buf_.has_msg();
	}

	NetDataHeader* front_msg()
	{
		return (NetDataHeader*)recv_buf_.data();
	}

	void pop()
	{
		if(has_msg())
			recv_buf_.pop(front_msg()->length_);
	}

	bool need_write()
	{
		return send_buf_.need_write();
	}

	// 立即将缓冲区数据发送出去
	int send_now()
	{
		reset_send();
		return send_buf_.write_socket(sockfd_);
	}

	// 缓冲区的控制根据业务需求的差异而调整
	// 发送数据给指定的客户端
	int send_data(NetDataHeader *head)
	{
		if (send_buf_.push((const char*)head, head->length_))
			return head->length_;
		
		return SOCKET_ERROR;
	}

	void reset_heart()
	{
		dt_heart_ = 0;
	}

	bool check_heart(time_t dt)
	{
		dt_heart_ += dt;
		if (dt_heart_ >= CLIENT_HEART_DEAD_TIME)
		{
			//CellLog::Info("check heat:socket=%d, time=%d\n", sockfd_, dt_heart_);
			return true;
		}
		return false;
		//return dt_heart_ >= CLIENT_HEART_DEAD_TIME;
	}

	void reset_send()
	{
		dt_send_ = 0;
	}

	bool check_send(time_t dt)
	{
		dt_send_ += dt;
		if (dt_send_ >= CLIENT_SEND_TIME)
		{
			//CellLog::Info("check send:socket=%d, time=%d\n", sockfd_, dt_send_);
			// 立即发送缓冲区的数据
			send_now();
			reset_send();
			return true;
		}
		return false;
	}

public:
	int id = -1;
	int server_id = -1;

private:
	SOCKET sockfd_;						// socket fd_set file desc set
	CellBuffer send_buf_;				// 发送缓冲区
	CellBuffer recv_buf_;				// 接受缓冲区
	int send_buf_count_ = 0;			// 缓冲区写满的次数
	time_t dt_heart_;					// 心跳死亡计时
	time_t dt_send_;					// 上次发送消息的时间
};

#endif  // !CELL_CLIENT_HPP_