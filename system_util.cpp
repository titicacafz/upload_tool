#include <Windows.h>
#include <chrono>
#include "system_util.h"
#include <filesystem>

std::string SystemUtil::string_to_utf8(const std::string & str) {
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];
	ZeroMemory(pwBuf, nwLen * 2 + 2);

	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);

	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr(pBuf);

	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;

	return retStr;
}

std::string SystemUtil::bytes_to_jsonarray(const char* bytes, const int length)
{
	if (bytes == NULL) {
		return "";
	}
	std::string buff;
	const int len = length;
	buff += "[";
	for (int i = 0; i < len; i++) {
		buff += std::to_string((unsigned char)bytes[i]);
		if (i + 1 < len)
			buff += ",";
	}
	buff += "]";
	return buff;

}

std::string SystemUtil::utf8_to_string(const std::string & str) {
	int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

	wchar_t * pwBuf = new wchar_t[nwLen + 1];
	memset(pwBuf, 0, nwLen * 2 + 2);

	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);

	int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

	char * pBuf = new char[nLen + 1];
	memset(pBuf, 0, nLen + 1);

	WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

	std::string retStr = pBuf;

	delete[]pBuf;
	delete[]pwBuf;

	pBuf = NULL;
	pwBuf = NULL;

	return retStr;
}


std::string SystemUtil::get_module_file()
{
	char szBuff[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, szBuff, MAX_PATH);
	std::string path{ szBuff };
	return path;
}

//获得dll所在路径
std::string SystemUtil::get_module_path()
{
	char szBuff[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, szBuff, MAX_PATH);
	std::string path{ szBuff };
	auto found = path.find_last_of('\\');
	path = path.substr(0, found+1);
	return path;
}

std::string SystemUtil::get_module_conf()
{
	char szBuff[MAX_PATH] = { 0 };
	::GetModuleFileName(NULL, szBuff, MAX_PATH);
	std::string conf{ szBuff };
	auto found = conf.find_last_of('\\');
	string_replace(conf, ".exe", ".conf");
	string_replace(conf, ".dll", ".conf");
	return conf;
}

std::string SystemUtil::get_date(int day_pos)
{
	auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tt += day_pos * 60 * 60 * 24;
	struct tm t; 
	localtime_s(&t, &tt);
	char date[11] = { 0 };
	snprintf(date, sizeof(date),"%d-%02d-%02d",(int)t.tm_year + 1900, (int)t.tm_mon + 1, (int)t.tm_mday);
	return std::string(date);
}

void SystemUtil::string_replace(std::string& strBase, std::string strSrc, std::string strDes)
{
	std::string::size_type pos = 0;
	std::string::size_type srcLen = strSrc.size();
	std::string::size_type desLen = strDes.size();
	pos = strBase.find(strSrc, pos);
	while ((pos != std::string::npos))
	{
		strBase.replace(pos, srcLen, strDes);
		pos = strBase.find(strSrc, (pos + desLen));
	}
}

void SystemUtil::uri_parse(const std::string& uri, std::string& host, int& port, std::string& path)
{
	std::string temp;
	size_t start_pos = 0;
	auto pos = uri.find("://", start_pos);
	if (pos != std::string::npos) {
		temp = uri.substr(start_pos, pos);
		start_pos = pos + 3;
	}

	pos = uri.find(":", start_pos);

	if (pos != std::string::npos) {
		host = uri.substr(start_pos, pos - start_pos);
		start_pos = pos + 1;

		pos = uri.find("/", start_pos);
		if (pos != std::string::npos) {
			port = stoi(uri.substr(start_pos, pos - start_pos));
			start_pos = pos;
		}
	}
	else
	{
		pos = uri.find("/", start_pos);
		host = uri.substr(start_pos, pos - start_pos);
		start_pos = pos + 1;
		port = 80;
	}

	

	path = uri.substr(start_pos);
}

//获取日志目录
std::string SystemUtil::get_log_file()
{
	std::string log_dir = get_module_path() + "Log\\";
	namespace fs = std::filesystem;
	fs::create_directories(log_dir);
	log_dir += "upload-" + get_date() + ".log";

	return log_dir;
}
