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
#ifdef _DEBUG
	#ifndef CELLLOG_DEBUG
	#define CELLLOG_DEBUG(...) CellLog::debug(__VA_ARGS__)
	#endif // !1
#else
	#ifndef CELLLOG_DEBUG
	#define CELLLOG_DEBUG(...) 
	#endif // !1
#endif
//#define CELLLOG_DEBUG(...) CellLog::debug(__VA_ARGS__)
#define CELLLOG_INFO(...)  CellLog::info(__VA_ARGS__)
#define CELLLOG_WARN(...)  CellLog::warning(__VA_ARGS__)
#define CELLLOG_ERROR(...) CellLog::error(__VA_ARGS__)


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

	void set_path(const char* name, const char* mode)
	{
		if (log_file_)
		{
			fclose(log_file_);
			log_file_ = nullptr;
			info("CellLog::set_path fclose");

		}

		static char log_path[512] = {};
		auto t = system_clock::now();
		auto tnow = system_clock::to_time_t(t);
		std::tm* now = std::localtime(&tnow);
		sprintf(log_path, "%s[%d-%d-%d %d-%d-%d].txt", name,
			now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

		log_file_ = fopen(log_path, mode);
		if (log_file_)
		{
			info("CellLog::set_path success<%s,%s>", log_path, mode);
		}
		else
		{
			info("CellLog::set_path failed<%s,%s>", log_path, mode);
		}
	}

	

	static void error(const char* pstr)
	{
		error("%s", pstr);
	}
	template<typename ... Args>
	static void error(const char* format, Args ... args)
	{
		echo("error", format, args...);
	}


	static void warning(const char* pstr)
	{
		warning("%s", pstr);
	}
	template<typename ... Args>
	static void warning(const char* format, Args ... args)
	{
		echo("warning",format, args...);
	}


	static void debug(const char* pstr)
	{
		debug("%s", pstr);
	}
	template<typename ... Args>
	static void debug(const char* format, Args ... args)
	{
		echo("debug",format, args...);
	}


	static void info(const char* pstr)
	{
		info("%s", pstr);
	}
	template<typename ... Args>
	static void info(const char* format, Args ... args)
	{
		echo("info",format, args...);
	}

	template<typename ... Args>
	static void echo(const char* level, const char* format, Args ... args)
	{
		CellLog* plog = &Instance();
		plog->task_server_.add_task([=]() 
		{
			if (plog->log_file_)
			{
				auto t = system_clock::now();
				auto tnow = system_clock::to_time_t(t);
				//fprintf(plog->log_file_, "%s", ctime(&now));
				std::tm* now = std::localtime(&tnow);
				fprintf(plog->log_file_, "[%s] ", level);
				fprintf(plog->log_file_, "[%d-%d-%d %d:%d:%d] ",
					now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				fprintf(plog->log_file_, format, args...);
				fprintf(plog->log_file_, "%s", "\n");
				fflush(plog->log_file_);
			}
			//if (sizeof...(Args))
			printf("%s:", level);
			printf(format, args...);
			printf("\n");
		});

		
	}
private:
	FILE* log_file_;
	CellTaskServer task_server_;
	
};



#endif // !CELL_LOG_HPP_