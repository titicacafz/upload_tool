#pragma once
#include <string>


struct FileDTO
{
	//��ʼλ��
    unsigned long StartPosition;
	//�ļ���С
	uintmax_t FileSize;
	//�Ƿ������ֻ���
	bool IsClient = false;
	//base64���ܵ��ֽ��ַ���
	std::string FileData;
	//�ϴ��ļ���
	std::string UploadFileName;

	std::string to_json();
};
