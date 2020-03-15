#ifndef CELL_BUFFER_HPP_
#define CELL_BUFFER_HPP_

/**
* @file cell_buffer.hpp
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-03-09
* @author morris
* contact: tappanmorris@outlook.com
*
*/
#include "cell.hpp"

class CellBuffer
{
public:
	CellBuffer(int size=8192) 
	{
		size_ = size;
		buffer_ = new char[size_];
	}
	~CellBuffer() 
	{
		if (buffer_)
		{
			delete[] buffer_;
			buffer_ = nullptr;
		}
			
	}

	char* data()
	{
		return buffer_;
	}

	bool push(const char* pdata, int len)
	{
		//if (last_ + length > size_)
		//{
		//	// 写入大量数据不一定要放到内存中
		//	// 也可以存储到数据库或者磁盘
		//	// 需要写入的数据大于可用空间
		//	int n = last_ + length - size_;
		//	if (n < 8192)
		//		n = 8192;
		//	char* buff = new char[size_ + n];
		//	memcpy(buff, buffer_, last_);
		//	delete[] buffer_;
		//	buffer_ = buff;
		//}
		if (last_ + len <= size_)
		{
			// 将要发送的数据拷贝到发送缓冲区尾部
			memcpy(buffer_ + last_, pdata, len);
			// 数据尾部位置
			last_ += len;
			
			if (last_ == size_)
				full_count_++;
			return true;
		}
		else
		{
			++full_count_;
		}
		return false;
	}

	void pop(int len)
	{
		int n = last_ - len;
		if (n > 0)
		{
			memcpy(buffer_, buffer_ + len, n);
		}
		last_ = n;
		if (full_count_ > 0)
			--full_count_;

	}


	int write_to_socket(SOCKET sockfd)
	{
		int ret = 0;
		if (last_ > 0)
		{
			ret = send(sockfd, buffer_, last_, 0);
			if(ret<=0)
			{
				CellLog::error("write2socket1:sockfd<%d> nSize<%d> nLast<%d> ret<%d>", sockfd, size_, last_, ret);
				return SOCKET_ERROR;
			}
			else if (ret == last_)
			{
				// 数据尾部清零
				last_ = 0;
			}
			else
			{
				//last_=2000 实际发送ret=1000
				last_ -= ret;
				memcpy(buffer_, buffer_ + ret, last_);
			}
			full_count_ = 0;
		}
		return ret;
	}

	int read_for_socket(SOCKET sockfd)
	{
		
		if (size_ - last_ <= 0)
			return 0;
			
		char *recv_buf = buffer_ + last_;
		int len = (int)recv(sockfd, recv_buf, size_ - last_, 0);
		if (len <= 0)
		{
			CELLLOG_ERROR("read4socket:sockfd<%d> nSize<%d> nLast<%d> nLen<%d>", sockfd, size_, last_, len);
			return SOCKET_ERROR;
		}
		last_ += len;
		return  len;
	}

	bool has_msg()
	{
		if (last_ >= sizeof(NetDataHeader))
		{
			// 这时候就知道了当前消息的长度
			NetDataHeader *head = (NetDataHeader*)buffer_;
			// 判断消息缓冲区的数据长度大于消息长度
			return (last_ >= head->length_);
		}
		return false;
	}
	
	bool need_write()
	{
		return last_>0;
	}

private:
	// 可以用链表和队列管理缓存
	//list<char*> list_buffer_;
	char* buffer_;  
	int last_ = 0;	// 缓冲区实际数据的长度
	int size_ = 0;		// 缓冲区字节长度
	int full_count_ = 0;
};





#endif  // CELL_BUFFER_HPP_