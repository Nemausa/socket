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
	CellBuffer(int size) 
	{
		size_ = size;
		buffer_ = new char[size_];
		last_ = 0;
		full_count_ = 0;
	}
	~CellBuffer() 
	{
		if (buffer_)
			delete[] buffer_;
	}

	char* data()
	{
		return buffer_;
	}

	bool push(const char* pdata, int length)
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
		if (last_ + length <= size_)
		{
			// 将要发送的数据拷贝到发送缓冲区尾部
			memcpy(buffer_ + last_, pdata, length);
			// 数据尾部位置
			last_ += length;
			if (last_ == size_)
				full_count_++;
			return true;
		}
		else
		{
			full_count_++;
		}
		return false;
	}

	void pop(int length)
	{
		int n = last_ - length;
		if (n > 0)
		{
			memcpy(buffer_, buffer_ + length, n);
		}
		last_ = n;
		if (full_count_ > 0)
			--full_count_;

	}


	int write_socket(SOCKET sockfd)
	{
		int ret = 0;
		if (last_ > 0)
		{
			ret = send(sockfd, buffer_, last_, 0);
			last_ = 0;
			full_count_ = 0;
		}
		return ret;
	}

	int read_socket(SOCKET sockfd)
	{
		
		if (size_ - last_ < 0)
			return 0;
		char *recv_buf = buffer_ + last_;
		int len = (int)recv(sockfd, recv_buf, size_ - last_, 0);
		if (len <= 0)
		{

			return len;
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
		return last_;
	}

private:
	// 可以用链表和队列管理缓存
	//list<char*> list_buffer_;
	char* buffer_;  
	int last_;		// 缓冲区尾部位置
	int size_;		// 缓冲区字节长度
	int full_count_;
};





#endif  // CELL_BUFFER_HPP_