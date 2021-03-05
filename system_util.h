#pragma once

#include <string>

//��ʱ
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
    //utf8 ת gbk
    static std::string utf8_to_string(const std::string & str);

    //gbk ת utf8
    static std::string string_to_utf8(const std::string & str);

    //����ת�ַ�����ʽ��json����
	static std::string bytes_to_jsonarray(const char* bytes, const int length);
   
    //
    static std::string get_module_file();
    //��ó����dll����·��
    static std::string get_module_path();

    //��ó����dll�����ļ���
    static std::string get_module_conf();

    //��õ�ǰ����
    static std::string get_date(int day_pos = 0);

    static void string_replace(std::string& strBase, std::string strSrc, std::string strDes);

    //http url������Config��
    static void uri_parse(const std::string& uri, std::string& host, int& port, std::string& path);

    //��ȡ��־Ŀ¼
    static std::string get_log_file();
};
