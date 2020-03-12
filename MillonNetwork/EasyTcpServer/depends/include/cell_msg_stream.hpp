﻿#ifndef CELL_RECV_STREAM_HPP_
#define CELL_RECV_STREAM_HPP_

/**
* @file cell_stream.hpp
*
* @brief byte stream
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-03-11
* @author morris
* contact: tappanmorris@outlook.com
*
*/
#include "message.hpp"
#include "cell_stream.hpp"

#include <cstdint>

class CellReadStream:public CellStream
{
public:
	CellReadStream(NetDataHeader* head)
		:CellStream((char*)head, head->length_)
	{
		push(head->length_);
		
	}

	CellReadStream(char* pdata, int size, bool b_delete = false)
		:CellStream(pdata, size, b_delete)
	{
		
	}

	CellReadStream(int size = 1024)
		:CellStream(size)
	{
		
	}

	uint16_t get_cmd()
	{
		uint16_t cmd = CMD_ERROR;
		read<uint16_t>(cmd);
		write<uint16_t>(cmd);
		return cmd;
	}

};


class CellWriteStream :public CellStream
{
public:
	CellWriteStream(char* pdata, int size, bool b_delete = false)
		:CellStream(pdata, size, b_delete)
	{
		// 预先占领消息长度所需空间
		write<int16_t>(0);
	}

	CellWriteStream(int size = 1024)
		:CellStream(size)
	{
		// 预先占领消息长度所需空间
		write<int16_t>(0);
	}

	void finish()
	{
		int pos = length();
		setWritePos(0);
		write<uint16_t>(pos);
		setWritePos(pos);
	}

	void set_cmd(uint16_t cmd)
	{
		write<int16_t>(cmd);
	}
};



#endif  // !CELL_RECV_STREAM_HPP_