/*
 * @Author: 饕餮
 * @Date: 2022-05-01 13:08:37
 * @version: 
 * @LastEditors: SpenserCai
 * @LastEditTime: 2023-02-08 16:26:47
 * @Description: file content
 */
#include "PanGuService.h"
#include <WtsApi32.h>
// #include "json/json.h" 

#pragma comment(lib,"Wtsapi32.lib")

void PanGuService::DoWork() { 
    //获取用户名
    string userName = ServiceUtils::getUserName();
    //获取当前时间
    string currentTime = ServiceUtils::getCurrentTime(); 
    // TODO:检查定时任务，如果到执行时间创建一个新的线程取执行任务
    // if (ServiceUtils::isProcessExist("PanGuClient.exe")) {
    //     std::cout<<"PanGuClient.exe is exist"<<std::endl;
    // }
    // else {
    //     std::string userDataPath = std::string(getenv("SystemDrive")) + "\\Users\\" + ServiceUtils::GetActiveUserName() + std::string("\\AppData\\Roaming\\pangu-client");
    //     // 判断%AppData%/pangu-client/Network/NetworkDataExport是否存在
    //     if (!ServiceUtils::isFileExist(userDataPath + std::string("\\Network\\NetworkDataExport")))
    //     {
    //         std::string  strAppName = std::string(getenv("ProgramFiles")) + std::string("\\PanGuClient\\") +  "PanGuClient.exe";
	//         std::vector<std::string> params = {};
	//         ServiceUtils::launchGUIApplication(strAppName, params);
    //     }
    // }
}

bool PanGuService::TestSendStatus(Json::Value& data)
{
    std::cout<<data.toStyledString()<<std::endl;
    return true;
}

void PanGuService::OnStart(DWORD,TCHAR**) {
    isWork = true;
    pluginUtil = new PluginUtils();
    serviceRpc.pluginUtils = (std::shared_ptr<PluginUtils>)pluginUtil;
    // 轮询检查是否有定时任务
    std::thread workThread([this]() {
        while (isWork) {
            DoWork();
            Sleep(6000);
        }
    });
    workThread.detach();
    // 安全检测线程
    std::thread safeThread([this]() {
        while (isWork) {
            ServiceUtils::antiHacker();
            Sleep(1000);
        }
    });
    safeThread.detach();
    // 启动grpc线程
    grpc_thread = std::thread([this]() {
        serviceRpc.RunServer();
    });
    grpc_thread.detach();
}

void PanGuService::OnStop() {
    isWork = false;
    // 停止grpc服务
    serviceRpc.StopServer();

}

