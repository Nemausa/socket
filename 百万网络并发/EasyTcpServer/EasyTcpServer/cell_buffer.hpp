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
		buffer_full_count_ = 0;
	}
	~CellBuffer() 
	{
		if (buffer_)
			delete[] buffer_;
	}

	// 立即将缓冲区数据发送出去
	int write_socket(SOCKET sockfd)
	{
		int ret = 0;
		if (last_ > 0)
		{
			ret = send(sockfd, buffer_, last_, 0);
			last_ = 0;
			buffer_full_count_ = 0;
		}
		return ret;

	}

	bool push(const char* pdata, int length)
	{
		if (last_ + length <= size_)
		{
			// 将要发送的数据拷贝到发送缓冲区尾部
			memcpy(buffer_ + last_, pdata, length);
			// 数据尾部位置
			last_ += length;
			if (last_ == size_)
				buffer_full_count_++;
			return true;
		}
		else
		{
			buffer_full_count_++;
		}
		return false;
	}

private:
	char* buffer_;  
	int last_;		// 缓冲区尾部位置
	int size_;		// 缓冲区字节长度
	int buffer_full_count_;
};





#endif  // CELL_BUFFER_HPP_