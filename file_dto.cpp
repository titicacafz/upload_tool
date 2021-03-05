#include "file_dto.h"
#include "system_util.h"
using namespace std;

string FileDTO::to_json()
{
    string json;
    //{"fileDTO":{
    //"FileData": [50, 49, 51, 49, 51, 49] , 
    //"FileDataFromPhone" : null, 
    //"FilePath" : "", 
    //"FileSize" : 6, 
    //"FolderName" : "", 
    //"IsClient" : false, 
    //"NewAppId" : null, 
    //"StartPosition" : 0, 
    //"StorageTime" : 0, 
    //"UploadFileName" : "1.txt"}}
    json.append("{\"fileDTO\":{");
    std::string json_data = SystemUtil::bytes_to_jsonarray(FileData.c_str(), FileData.length());
    json.append("\"FileData\":" + json_data+",");
    json.append("\"FileDataFromPhone\":null,");
    json.append("\"FilePath\":\"\",");
    json.append("\"FileSize\":" + std::to_string(FileSize) + ",");
    json.append("\"FolderName\":\"\",");
    json.append("\"IsClient\":false,");
    json.append("\"NewAppId\":null,");
    json.append("\"StartPosition\":0,");
    json.append("\"StorageTime\":0,");
    json.append("\"UploadFileName\":\"" + UploadFileName+"\"");
    
    json.append("}}");
    return json;
}
