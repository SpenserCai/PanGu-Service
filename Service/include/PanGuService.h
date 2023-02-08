/*
 * @Author: 饕餮
 * @Date: 2022-05-01 13:08:53
 * @version: 
 * @LastEditors: SpenserCai
 * @LastEditTime: 2023-02-08 15:01:34
 * @Description: file content
 */
#ifndef PANGU_SERVICE_H
#define PANGU_SERVICE_H

#include <fstream>
#include <thread>
#include "service_base.h"
#include "ServiceModule/ServiceUtils.h"
#include "ServiceModule/ServiceLocalRpc.h"
#include "ServiceModule/PluginUtils.h"

class PanGuService : public ServiceBase {
    public:
        ServiceLocalRpc serviceRpc;
        PluginUtils* pluginUtil;
        PanGuService(const PanGuService& other) = delete;
        PanGuService& operator=(const PanGuService& other) = delete;
        bool TestSendStatus(Json::Value& data);
        PanGuService(PanGuService&& other) = delete;
        PanGuService& operator=(PanGuService&& other) = delete;
        // 手动启动
        PanGuService() : ServiceBase(_T("PanGuService"), _T("PanGu Service"), SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, SERVICE_ACCEPT_SESSIONCHANGE) {}
        //PanGuService() : ServiceBase(_T("PanGuService"), _T("PanGu Service"), SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,  SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SESSIONCHANGE) {}

    private:
        void OnStart(DWORD argc, TCHAR* argv[]) override;
        void OnStop() override;
        //void OnSessionChange(DWORD evtType,WTSSESSION_NOTIFICATION* notification) override;
        void DoWork();
        BOOL isWork;
        // 维持grpc的线程
        std::thread grpc_thread;

};

#endif  // PANGU_SERVICE_H