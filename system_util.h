#pragma once

#include <string>

//临时
template<typename T>
class Singleton {

public:

	//
	static T& Instance() {   //
		static T t;
		return t;
	}

	Singleton(const Singleton &another) = delete;

private:
	Singleton();
};


struct  SystemUtil
{
    //utf8 转 gbk
    static std::string utf8_to_string(const std::string & str);

    //gbk 转 utf8
    static std::string string_to_utf8(const std::string & str);

    //数组转字符串形式的json数组
	static std::string bytes_to_jsonarray(const char* bytes, const int length);
   
    //
    static std::string get_module_file();
    //获得程序或dll所在路径
    static std::string get_module_path();

    //获得程序或dll配置文件名
    static std::string get_module_conf();

    //获得当前日期
    static std::string get_date(int day_pos = 0);

    static void string_replace(std::string& strBase, std::string strSrc, std::string strDes);

    //http url解析到Config中
    static void uri_parse(const std::string& uri, std::string& host, int& port, std::string& path);

    //获取日志目录
    static std::string get_log_file();
};
