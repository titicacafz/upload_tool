#include "http_connect.h" 
#ifdef WIN32
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif
#include <sstream>


HttpConnect::HttpConnect()
{
	WSADATA wsa = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsa);
}


HttpConnect::~HttpConnect()
{
}

bool HttpConnect::socketHttp(std::string host, int port, std::string request, std::string &response)
{
	bool bret = false;
	int sockfd;
    
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	//发送和接收超时都设置成2秒
	int send_timeout = 30000;// Global.config.send_timeout * 1000;
	int recv_timeout = 30000;// Global.config.recv_timeout * 1000;
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&send_timeout, sizeof(send_timeout));
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeout, sizeof(recv_timeout));

	// 设置为非阻塞的socket  
	unsigned long sock_mode = 1;
	ioctlsocket(sockfd, FIONBIO, (u_long FAR*)&sock_mode);
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	hostent *server = gethostbyname(host.c_str());
	memcpy((char *)&address.sin_addr.s_addr, (char*)server->h_addr, server->h_length);
	
    //异步sock直接返回-1
	int conn_ret = connect(sockfd, (struct sockaddr *)&address, sizeof(address));
	timeval tv;
	fd_set r;
	FD_ZERO(&r);
	FD_SET(sockfd, &r);
	tv.tv_sec = 30;// Global.config.conn_timeout;
	tv.tv_usec = 0;
	int ret = select(0, 0, &r, 0, &tv);
	if (ret <= 0)
	{
		closesocket(sockfd);
        if (ret == 0) {
            throw std::exception{ "连接超时" };
        }
		return false;
	}

	sock_mode = 0;
	ioctlsocket(sockfd, FIONBIO, (u_long FAR*)&sock_mode); //设置为阻塞模式  

    //int Global.config = 10 * 1000;
    //int sendTimeout = 10 * 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeout, sizeof(int));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&send_timeout, sizeof(int));
    //

#ifdef WIN32    
	send(sockfd, request.c_str(), request.size(), 0);    
#else
	write(sockfd, request.c_str(), request.size());
#endif
	//固定1K，每次读1K，循环读完为止
	char buf[1024+1]{0};
	int rc = 0;

#ifdef WIN32
	while (rc = recv(sockfd, buf, 1024, 0))
#else
	while (rc = read(sockfd, buf, 1024))
#endif
	{
		if (rc < 0) {           
			return false;
		}
		buf[rc] = 0;        
		response.append(std::string(buf));
	}

#ifdef WIN32
	closesocket(sockfd);
#else
	close(sockfd);
#endif
	//delete [] buf;
	//只取body部分
	if (response.find("200 OK") != std::string::npos) {
		bret = true;
	}

	if (response.find("\r\n\r\n") != std::string::npos) {
		response = response.substr(response.find("\r\n\r\n")+4);
	}
	return bret;
}

bool HttpConnect::postData(std::string host, int port,std::string path, std::string post_content, std::string &response)
{
	//POST请求方式	
	std::stringstream stream;
	stream << "POST " << "/" <<path <<" ";
	stream << "HTTP/1.1\r\n";
	stream << "Host: " << host << "\r\n";
	//stream << "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3\r\n";
	stream << "Content-Type:application/json\r\n";
	stream << "charset:UTF-8\r\n";
	stream << "Content-Length:" << post_content.length() << "\r\n";
	stream << "Connection:close\r\n\r\n";
	stream << post_content.c_str();

	return socketHttp(host, port,stream.str(), response);
}
