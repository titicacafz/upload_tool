#include <Windows.h>
#include <filesystem> 
#include "config_file.h"
#include <fstream>
#include <sstream>
#include "system_util.h"
#include "file_dto.h"
#include "http_connect.h"
#include "json.hpp"
#include "logger.h"
#include <thread>
#include <iostream>

using namespace std;
using namespace std::filesystem;


#define SAFE_CALL(FuncCall, ErrorCode)		                        \
	if (FuncCall == ErrorCode) {			                        \
		cout << #FuncCall " error, code:" << GetLastError()         \
             << " ,line:" << __LINE__ << "\n"; 		                \
		exit(-1);							                        \
	}

/////////////////////////////////////////////////////////////////////////////////////////////////

void installService()
{
    auto scmHandle = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    SAFE_CALL(scmHandle, NULL);

    auto serviceHandle = CreateService(scmHandle,
        "UploadFiles::Service",
        "UploadFiles::Service",
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        SystemUtil::get_module_file().c_str(),
        NULL, NULL, "", NULL, "");
    SAFE_CALL(serviceHandle, NULL);

    CloseServiceHandle(scmHandle);
    CloseServiceHandle(serviceHandle);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void uninstallService()
{
    auto scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    SAFE_CALL(scmHandle, NULL);

    auto serviceHandle = OpenService(scmHandle,
        "UploadFiles::Service",
        SERVICE_ALL_ACCESS);
    SAFE_CALL(serviceHandle, NULL);

    SERVICE_STATUS serviceStatus;
    SAFE_CALL(QueryServiceStatus(serviceHandle, &serviceStatus), 0);
    if (serviceStatus.dwCurrentState == SERVICE_RUNNING) {
        SAFE_CALL(ControlService(serviceHandle, SERVICE_CONTROL_STOP, &serviceStatus), 0);
        SAFE_CALL(serviceStatus.dwCurrentState, NO_ERROR);

        do {
            SAFE_CALL(QueryServiceStatus(serviceHandle, &serviceStatus), 0);
            Sleep(1000);
        } while (serviceStatus.dwCurrentState != SERVICE_STOPPED);
    }

    SAFE_CALL(DeleteService(serviceHandle), FALSE);

    CloseServiceHandle(scmHandle);
    CloseServiceHandle(serviceHandle);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void startService()
{
    auto scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    SAFE_CALL(scmHandle, NULL);

    auto serviceHandle = OpenService(scmHandle,
        "UploadFiles::Service",
        SERVICE_ALL_ACCESS);
    SAFE_CALL(serviceHandle, NULL);

    SERVICE_STATUS serviceStatus;
    SAFE_CALL(QueryServiceStatus(serviceHandle, &serviceStatus), 0);
    if (serviceStatus.dwCurrentState == SERVICE_START &&
        serviceStatus.dwCurrentState != SERVICE_START_PENDING)
        return;

    SAFE_CALL(StartService(serviceHandle, 0, NULL), FALSE);

    CloseServiceHandle(scmHandle);
    CloseServiceHandle(serviceHandle);
}

SERVICE_STATUS_HANDLE g_serviceStatusHandle = NULL;

void setServiceStatus(DWORD status)
{
    SERVICE_STATUS serviceStatus;
    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwWin32ExitCode = NO_ERROR;
    serviceStatus.dwServiceSpecificExitCode = 0;
    serviceStatus.dwWaitHint = 2000;
    serviceStatus.dwCheckPoint = 0;
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_PAUSE_CONTINUE |
        SERVICE_ACCEPT_SHUTDOWN |
        SERVICE_ACCEPT_STOP;

    serviceStatus.dwCurrentState = status;
    SAFE_CALL(SetServiceStatus(g_serviceStatusHandle, &serviceStatus), 0);
}

VOID WINAPI ServiceHandler(DWORD controlCode)
{
    switch (controlCode)
    {
    case SERVICE_CONTROL_CONTINUE:
        setServiceStatus(SERVICE_START_PENDING);    
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    case SERVICE_CONTROL_PAUSE:
        setServiceStatus(SERVICE_PAUSED);           
        break;
    case SERVICE_CONTROL_SHUTDOWN:
        setServiceStatus(SERVICE_STOPPED);          
        break;
    case SERVICE_CONTROL_STOP:
        setServiceStatus(SERVICE_STOPPED);          
        break;
    default:
        break;
    }
}
void run();
VOID WINAPI ServiceMain(DWORD argc, LPSTR* argv)
{
    g_serviceStatusHandle = RegisterServiceCtrlHandler("UploadFiles::Service", &ServiceHandler);
    if (g_serviceStatusHandle == 0)
    {
        cout << "RegisterServiceCtrlHandlerW error, code:" << GetLastError()
            << " ,line:" << __LINE__ << "\n";
        exit(-1);
    }

    setServiceStatus(SERVICE_START_PENDING);
    setServiceStatus(SERVICE_RUNNING);
    run();
}

void runService()
{
    const SERVICE_TABLE_ENTRY serviceTable[] = {
        { "", ServiceMain },
        { NULL, NULL }
    };

    SAFE_CALL(StartServiceCtrlDispatcher(&serviceTable[0]), 0);
}

int main(int argc, char* argv[])
{
    if (argc == 1)
        runService();
    else if (argc == 2) {
        if (argv[1] == string("-install"))
            installService(), startService();
        if (argv[1] == string("-uninstall"))
            uninstallService();
    }
    else
        std::cout << "usage: a.exe [-install/-uninstall]";

    return 0;
}


void run()
{
    while (true)
    {
        //配置文件要重新读
        string file_root = ConfigFile::read_conf("files", "file_root");
        string file_upload_server = ConfigFile::read_conf("files", "file_upload_server");
        string sleep_timeout = ConfigFile::read_conf("files", "sleep_timeout");
        int sec = atoi(sleep_timeout.c_str()) * 1000;

        string upload_dir;
        string upload_type = ConfigFile::read_conf("files", "upload_type");
        if (upload_type == "2") {//当天
            upload_dir = SystemUtil::get_date();
        }
        else if (upload_type == "1") {//前一天
            upload_dir = SystemUtil::get_date(-1);
        }  
        else
        {//指定日期
            upload_dir = upload_type;
        }

        file_root += "/"+upload_dir;

        path path_file_root(file_root);
        if (!exists(path_file_root)) {

            logger::record() << "上传目录不存在：" << file_root << std::endl;
            this_thread::sleep_for(std::chrono::milliseconds(sec));
            continue;
        }

        directory_iterator files_list(path_file_root);
        
        int i = 0;

        for (auto& it : files_list)
        {
            
            i++;
            string upload_file = it.path().string();
            string upload_record = SystemUtil::get_module_path() + upload_dir + "record.txt";
            //要判断文件是否已经上传过
            
            string upload_files;
            ifstream upload_reocde_stream(upload_record, ios::in);
            if (upload_reocde_stream.is_open())
            {
                upload_reocde_stream.seekg(0, ios::end);
                auto len = upload_reocde_stream.tellg();
                upload_files.resize(len);
                upload_reocde_stream.seekg(0, std::ios::beg);
                upload_reocde_stream.read(reinterpret_cast<char*>(upload_files.data()), len);

                //已经上传过
                if (upload_files.find(upload_file) != string::npos) {
                    this_thread::sleep_for(std::chrono::milliseconds(sec));
                    continue;
                }
            }
            
            //
            logger::record() << "[" << std::endl;
            logger::record() << "准备第" << i << "个文件信息：" << upload_file << std::endl;
            FileDTO file_dto;
            file_dto.IsClient = false;
            file_dto.StartPosition = 0;
            file_dto.UploadFileName = it.path().filename().string();
            file_dto.FileSize = it.file_size();
            file_dto.FileData.resize(file_dto.FileSize);

            //logger::record() << "读取文件内容begin." << std::endl;
            ifstream file_data(upload_file, ios::binary);
            file_data.read(reinterpret_cast<char*>(file_dto.FileData.data()), file_dto.FileSize);
            //logger::record() << "读取文件内容end." << std::endl;

            string host;
            int port;
            string uri_path;

            SystemUtil::uri_parse(file_upload_server, host, port, uri_path);

            std::string response;
            HttpConnect http_conn;
            nlohmann::json json;
            logger::record() << "开始上传文件……" << std::endl;
            bool bret = http_conn.postData(host, port, uri_path, SystemUtil::string_to_utf8(file_dto.to_json()), response);
            response = SystemUtil::utf8_to_string(response);
            if (!bret) {
                logger::record() << "上传失败：" << response << std::endl;
            }
            else {

                logger::record() << "上传结束，收到服务器返回：" << response << std::endl;
                //成功要记录到上传列表文件
                
                ofstream out(upload_record, ios::out|ios::app);
                out << upload_file << endl;
            }

            logger::record() << "]" << std::endl;
        }
        
        this_thread::sleep_for(std::chrono::milliseconds(sec));
    }
   
    return;
}

