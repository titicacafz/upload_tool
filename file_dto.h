#pragma once
#include <string>


struct FileDTO
{
	//开始位置
    unsigned long StartPosition;
	//文件大小
	uintmax_t FileSize;
	//是否来自手机端
	bool IsClient = false;
	//base64加密的字节字符串
	std::string FileData;
	//上传文件名
	std::string UploadFileName;

	std::string to_json();
};
