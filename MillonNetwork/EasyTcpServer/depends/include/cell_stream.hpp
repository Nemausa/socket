#ifndef CELL_STREAM_HPP_
#define CELL_STREAM_HPP_

/**
* @file cell_stream.hpp
*
* @brief byte stream
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-03-10
* @author morris
* contact: tappanmorris@outlook.com
*
*/

#include <cstdint>

class CellStream
{
public:
	CellStream(char* pdata, int size, bool b_delete = false)
	{
		size_ = size;
		buffer_ = pdata;
		b_delete_ = b_delete;
	}

	CellStream(int size = 1024)
	{
		size_ = size;
		buffer_ = new char[size_];
		b_delete_ = true;
	}
	virtual ~CellStream()
	{
		if (b_delete_ && buffer_)
		{
			delete[] buffer_;
			buffer_ = nullptr;
		}
		
	}
public:

	inline bool can_read(int n)
	{
		return size_ - readpos_ >= n;
	}

	inline bool can_write(int n)
	{
		return size_ - writepos_ >= n;
	}

	//已写入位置，添加n字节长度
	inline void push(int n)
	{
		writepos_ += n;
	}

	//已读取位置，添加n字节长度
	inline void pop(int n)
	{
		readpos_ += n;
	}

	inline void setWritePos(int n)
	{
		writepos_ = n;
	}

	char* data()
	{
		return buffer_;
	}

	int length()
	{
		return writepos_;
	}



	template<typename T>
	bool read(T& n, bool offset=true)
	{
		auto nlen = sizeof(T);
		// 判断能不能读取
		if (can_read(nlen))
		{
			// 将要读取的数据拷贝出来
			memcpy(&n, buffer_ + readpos_, nlen);
			// 计算已经读取的位置
			if (offset)
				pop(nlen);
			return true;
		}
		return false;
	}

	template<typename T>
	uint32_t read_array(T* parr, uint32_t len)
	{
		// 读取数组的元素个数
		uint32_t nlen = 0;
		read(nlen, false);
		if (nlen <= len)
		{
			// 计算数组的字节长度
			auto size = sizeof(T)*nlen;
			if (can_read(size + sizeof(uint32_t)))
			{
				pop(sizeof(uint32_t));
				memcpy(parr, buffer_ + readpos_, size);
				// 计算已经读取的位置
				pop(size);
				return nlen;
			}
		}
		CELLLOG_ERROR("CellStream::ReadArray failed.");
		return 0;
	}


	template<typename T>
	bool write(T n)
	{
		size_t nlen = sizeof(T);
		// 判断能不能写入
		if (can_write(nlen))
		{
			memcpy(buffer_ + writepos_, &n, nlen);
			push(nlen);
			return true;
		}
		return false;
	}

	template<typename T>
	bool write_array(T* pdata, uint32_t len)
	{
		// 计算要写入数组的字节长度
		auto nlen = sizeof(T)*len;
		// 判断能不能写入
		if (can_write(nlen + sizeof(uint32_t)))
		{
			// 写入数组的长度
			write_int32(len);
			memcpy(buffer_ + writepos_, pdata, nlen);
			push(nlen);
			return true;
		}
		return false;
	}

	template<typename T>
	bool only_read(T& n)
	{
		return read(n, false);
	}

	//char size_t c# char2 char 1 
	int8_t read_int8(int8_t def = 0)
	{
		read(def);
		return def;
	}
	//short
	int16_t read_int16(int16_t n = 0)
	{
		read(n);
		return n;
	}
	//int
	int32_t read_int32(int32_t n = 0)
	{
		read(n);
		return n;
	}

	int64_t read_int64(int64_t n = 0)
	{
		read(n);
		return n;
	}

	uint8_t read_uint8(uint8_t def = 0)
	{
		read(def);
		return def;
	}
	//short
	uint16_t read_uint16(uint16_t n = 0)
	{
		read(n);
		return n;
	}
	//int
	uint32_t read_uint32(uint32_t n = 0)
	{
		read(n);
		return n;
	}

	uint64_t read_unit64(uint64_t n = 0)
	{
		read(n);
		return n;
	}

	float read_float(float n = 0.0f)
	{
		read(n);
		return n;
	}
	double read_double(double n = 0.0f)
	{
		read(n);
		return n;
	}


	// char
	bool write_int8(int8_t n) 
	{
		return write(n);
	}

	// short
	bool write_int16(int16_t n)
	{
		return write(n);
	}

	// int 
	bool write_int32(int32_t n)
	{
		return write(n);
	}

	bool write_float(float n)
	{
		return write(n);
	}

	bool write_double(double n)
	{
		return write(n);
	}
	



private:
	
	char* buffer_ = nullptr;	// 数据缓冲区
	int size_;					// 总字节长度
	int writepos_=0;	        // 已写入数据的大小
	int readpos_=0;				// 已读取数据的大小
	bool b_delete_;				// 是否需要删除
};



#endif  // !CELL_STREAM_HPP_