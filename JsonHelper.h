#pragma once
#include "json.hpp"


class JsonHelper
{
public:
	//��ȡjson�ڵ�ԭʼ�ַ�������ȥ������
	static std::string dump(const nlohmann::json& json)
	{
		std::string value;
		if (json.is_string()) {
			value = json.dump();
			value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
		}
		else {
			value = json.dump();
		}		

		return value;
	}

	//һ��ṹ��jsonת��xml
	static std::string xml(const nlohmann::json& json)
	{
		std::string xml;

	    for(auto & item : json.items() )
		{
			xml.append("<");
			xml.append(item.key());
			xml.append(">");
			xml.append(dump(item.value()));
			xml.append("</");
			xml.append(item.key());
			xml.append(">");
		}
		return xml;
	}
};
