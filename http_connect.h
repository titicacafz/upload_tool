#pragma once
#include <string>

class HttpConnect
{
public:
	HttpConnect();
	~HttpConnect();

	//
	bool HttpConnect::socketHttp(std::string host, int port, std::string request, std::string &response);

	//psot«Î«Û
	bool HttpConnect::postData(std::string host, int port, std::string path, std::string post_content, std::string &response);
};

