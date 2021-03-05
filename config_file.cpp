#include "config_file.h"
#include <fstream>
#include <iostream>
#include "system_util.h"

using std::string;
using std::map;


ConfigFile::ConfigFile()
{
}


ConfigFile::~ConfigFile()
{
}


std::string ConfigFile::read_conf(const std::string& sector, const std::string name)
{
	static ConfigFile conf_file;
	std::string inifile = SystemUtil::get_module_conf();
	std::map<std::string, std::string> fconfig;
	conf_file.read_config(inifile, fconfig, sector.c_str());
	return fconfig[name];
}

bool ConfigFile::read_config(const string& filename, map<string, string>& mContent, const char* section)
{
	mContent.clear();
	std::ifstream ins(filename.c_str());
	if (!ins)
	{
		//LOG4CXX_ERROR(logger, "file open error!");
		return false;
	}
	string line, key, value;
	int pos = 0;
	string Tsection = string("[") + section + "]";
	bool flag = false;
	while (getline(ins, line))
	{
		if (!flag)
		{
			pos = line.find(Tsection, 0);
			if (-1 == pos)
			{
				continue;
			}
			else
			{
				flag = true;
				getline(ins, line);
			}
		}
		if (0 < line.length() && '[' == line.at(0))
		{
			break;
		}
		if (0 < line.length() && analyse_line(line, key, value))
		{

			if (value.length() > 0)
			{
				if (value[value.size() - 1] == '\r')
				{
					value[value.size() - 1] = '\0';
				}
			}
			mContent[key] = value;
		}
	}
	ins.close();
	return true;
}

bool ConfigFile::analyse_line(const string & line, string & key, string & val)
{
	if (line.empty())
	{
		return false;
	}
	int start_pos = 0, end_pos = line.size() - 1, pos = 0;
	if ((pos = line.find(COMMENT_CHAR)) != -1)
	{
		if (0 == pos)
		{//行的第一个字符就是注释字符
			return false;
		}
		end_pos = pos - 1;
	}
	string new_line = line.substr(start_pos, start_pos + 1 - end_pos);  // 预处理，删除注释部分

	if ((pos = new_line.find('=')) == -1)
	{
		return false;  // 没有=号
	}

	key = new_line.substr(0, pos);
	val = new_line.substr(pos + 1, end_pos + 1 - (pos + 1));

	trim(key);
	if (key.empty())
	{
		return false;
	}
	trim(val);
	return true;
}

void ConfigFile::trim(string & str)
{
	if (str.empty())
	{
		return;
	}
	size_t i, start_pos, end_pos;
	for (i = 0; i < str.size(); ++i)
	{
		if (!is_space(str[i]))
		{
			break;
		}
	}
	if (i == str.size())
	{ //全部是空白字符串
		str = "";
		return;
	}

	start_pos = i;

	for (i = str.size() - 1; i >= 0; --i)
	{
		if (!is_space(str[i]))
		{
			break;
		}
	}
	end_pos = i;

	str = str.substr(start_pos, end_pos - start_pos + 1);
}

bool ConfigFile::is_space(char c)
{
	if (' ' == c || '\t' == c)
	{
		return true;
	}
	return false;
}

bool ConfigFile::is_comment_char(char c)
{
	switch (c)
	{
	case COMMENT_CHAR:
		return true;
	default:
		return false;
	}
}

void ConfigFile::print_config(const map<string, string> & mContent)
{
	map<string, string>::const_iterator mite = mContent.begin();
	for (; mite != mContent.end(); ++mite)
	{
		std::cout << mite->first << "=" << mite->second << std::endl;
	}
}
