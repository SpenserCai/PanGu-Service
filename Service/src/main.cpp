/*
 * @Author: SpenserCai
 * @Date: 2023-02-08 15:34:54
 * @version: 
 * @LastEditors: SpenserCai
 * @LastEditTime: 2023-02-08 15:52:25
 * @Description: file content
 */
#include "service_installer.h"
#include "PanGuService.h"
#include "ServiceModule/ServiceLocalRpc.h"
#include "ServiceModule/ServiceUtils.h"
#include "ServiceModule/PluginUtils.h"

// 主函数
int main(int argc, TCHAR* argv[]) {
    PanGuService service;
    if (argc > 1) {
        if (_tcscmp(argv[1],_T("install")) == 0) {
            std::cout<<"install service"<<std::endl;
            if (!ServiceInstaller::Install(service)) {
                std::cout<<"install service failed:"<<::GetLastError()<<std::endl;
                return -1;
              return -1;
            }
            std::cout<<"install service success"<<std::endl;
            return 0;
        }

        if (_tcscmp(argv[1],_T("uninstall")) == 0) {
            std::cout<<"uninstall service"<<std::endl;
            if (!ServiceInstaller::Uninstall(service)) {
                std::cout<<"uninstall service failed:"<<::GetLastError()<<std::endl;
                return -1;
            }
            std::cout<<"uninstall service success"<<std::endl;
            return 0;
        }

        // grpctest
        if (_tcscmp(argv[1],_T("grpctest")) == 0) {
            ServiceLocalRpc serviceRpc;
            PluginUtils* pluginUtils;
            pluginUtils = new PluginUtils();
            std::thread print_status_thread([&]() {
              while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                std::cout<<"RunningDataQueue size: "<<pluginUtils->RunningDataQueue.size()<<std::endl;
                //while (!pluginUtils->RunningDataQueue.empty()) {
                //  Json::Value data;
                //  data = pluginUtils->RunningDataQueue.front();
                //  pluginUtils->RunningDataQueue.pop();
                //  std::cout<<data<<std::endl;
                //}
              }
            });
            print_status_thread.detach();
            serviceRpc.pluginUtils = (std::shared_ptr<PluginUtils>)pluginUtils;
            serviceRpc.RunServer();
            // 启动一个线程，每隔10秒将pluginUtils->RunningDataQueue队列中的数据大印出来
            return 0;
        }
    }
    
    service.Run();

    return 0;
}

