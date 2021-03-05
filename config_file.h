#pragma once
#include <string>
#include <map>

#define COMMENT_CHAR '#'

class ConfigFile
{
public:
	ConfigFile();
	~ConfigFile();
	static std::string read_conf(const std::string& sector, const std::string name);
	bool read_config(const std::string& filename, std::map<std::string, std::string>& mContent, const char* section);
	bool analyse_line(const std::string & line, std::string & key, std::string & val);
	void trim(std::string & str);
	bool is_space(char c);
	bool is_comment_char(char c);
	void print_config(const std::map<std::string, std::string> & mContent);
};

