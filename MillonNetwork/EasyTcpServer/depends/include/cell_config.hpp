#ifndef CELL_CONFIG_HPP_
#define CELL_CONFIG_HPP_

/**
* @file cell_buffer.hpp
*
* @brief read configuration data 
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-03-015
* @author morris
* contact: tappanmorris@outlook.com
*
*/
#include <string>
#include <map>
#include "cell_log.hpp"

class CellConfig
{
private:
	CellConfig()
	{}
	~CellConfig()
	{}

public:
	static CellConfig& Instance()
	{
		static CellConfig sky;
		return sky;
	}

	void init(int argc, char* args[])
	{
		exe_path_ = args[0];
		for (int n = 1; n < argc; n++)
		{
			made_cmd(args[n]);
		}
	}

	void made_cmd(char* cmd)
	{
		// cmd ip=127.0.0.1
		char* val = strchr(cmd, '=');
		if (val)
		{
			// =127.0.0.1
			*val = '\0';
			// cmd的值 ip
			// \0127.0.0.1
			val++;
			// 127.0.0.1
			kv_[cmd] = val;
			CELLLOG_DEBUG("made cmd k<%s>|v<%s>", cmd, val);
		}
		else
		{
			kv_[cmd] = "";
			CELLLOG_DEBUG("made cmd k<%s>", cmd);
		}
	}

	const char* get_str(const char* key, const char* def)
	{
		auto iter = kv_.find(key);
		if (iter != kv_.end())
		{
			def = iter->second.c_str();
			CellLog::info("get_str, key=%s, value=%s", key, def);
		}
		else
		{
			CellLog::error("get_str, key=%s not found", key);
		}

		return def;
	}

	int get_int(const char* key, int def)
	{
		auto iter = kv_.find(key);
		if (iter != kv_.end())
		{
			def = atoi(iter->second.c_str());
			CellLog::info("get_int, key=%s, value=%d", key, def);
		}
		else
		{
			CellLog::error("get_int, key=%s not found", key);
		}

		return def;
	}

	bool exist_key(const char* key)
	{
		auto iter = kv_.find(key);
		return iter != kv_.end();
	}



private:
	// path of the current program
	std::string exe_path_;
	std::map<std::string, std::string> kv_;
};




#endif // !CELL_CONFIG_HPP_