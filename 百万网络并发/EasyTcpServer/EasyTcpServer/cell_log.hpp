#ifndef CELL_LOG_HPP_
#define CELL_LOG_HPP_

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
#include <stdio.h>
#include <ctime>
#include "cell_task.hpp"
#include "cell_time_stamp.hpp"

class CellLog
{
private:
	CellLog()
	{
		log_file_ = nullptr;
		task_server_.start();
	}

	~CellLog()
	{
		task_server_.close();
		fclose(log_file_);
		log_file_ = nullptr;
	}
public:
	static CellLog& Instance()
	{
		static CellLog log;
		return log;
	}

	void set_path(const char* path, const char* mode)
	{
		if (log_file_)
		{
			fclose(log_file_);
			log_file_ = nullptr;
			Info("CellLog::set_path fclose\n");

		}
					
		log_file_ = fopen(path, mode);
		if (log_file_)
		{
			Info("CellLog::set_path success<%s,%s>\n", path, mode);
		}
		else
		{
			Info("CellLog::set_path failed<%s,%s>\n", path, mode);
		}
	}

	//static void Info(const char* pstr)
	//{
	//	CellLog* plog = &Instance();
	//	plog->task_server_.add_task([plog, pstr]() {
	//		if (plog->log_file_)
	//		{
	//			auto t = system_clock::now();
	//			auto tnow = system_clock::to_time_t(t);
	//			//fprintf(plog->log_file_, "%s", ctime(&now));
	//			std::tm* now = std::gmtime(&tnow);
	//			fprintf(plog->log_file_, "[%d-%d-%d %d:%d:%d] ", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
	//			fprintf(plog->log_file_, "%s", pstr);
	//			fflush(plog->log_file_);
	//		}
	//		printf("%s", pstr);
	//	});
	//	

	//}
	template<typename ... Args>
	static void Info(const char* format, Args ... args)
	{
		CellLog* plog = &Instance();
		plog->task_server_.add_task([=]() 
		{
			if (plog->log_file_)
			{
				auto t = system_clock::now();
				auto tnow = system_clock::to_time_t(t);
				//fprintf(plog->log_file_, "%s", ctime(&now));
				std::tm* now = std::gmtime(&tnow);
				fprintf(plog->log_file_, "[%d-%d-%d %d:%d:%d] ", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				fprintf(plog->log_file_, format, args...);
				fflush(plog->log_file_);
			}
			if (sizeof...(Args))
				printf(format, args...);
			else
				printf("%s", format);
		});

		
	}
private:
	FILE* log_file_;
	CellTaskServer task_server_;
};



#endif // !CELL_LOG_HPP_