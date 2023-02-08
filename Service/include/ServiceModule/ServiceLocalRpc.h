/*
 * @Author: 饕餮
 * @Date: 2022-05-04 02:32:36
 * @version: 
 * @LastEditors: SpenserCai
 * @LastEditTime: 2023-02-08 14:20:13
 * @Description: 服务本地通讯库
 */
#ifndef SERVICE_LOCAL_RPC_H
#define SERVICE_LOCAL_RPC_H

#include <iostream>
#include <string>
#include <memory>
#include <fstream>

#include <grpc++/grpc++.h>
#include "pangu_service_rpc.grpc.pb.h"
#include <ServiceModule/PluginUtils.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::ServerAsyncWriter;
using grpc::ServerWriter;
using grpc::Status;
using grpc::experimental::Interceptor;
using pangu_service_rpc::BoolResponse;
using pangu_service_rpc::NullRequest;
using pangu_service_rpc::PanGuModuleService;
using pangu_service_rpc::ModuleList;
using pangu_service_rpc::ModuleCallRequest;
using pangu_service_rpc::ModuleCallResponse;
using pangu_service_rpc::ModuleRunningDataResponse;

// 插件服务
class PanGuModuleServiceImpl final : public PanGuModuleService::Service {
    public:
        // 一个用于设置PluginUtils的函数
        void setPluginUtils(std::shared_ptr<PluginUtils> pluginUtils);
        bool *isMainServiceRunning;

    // 一个用于存放PluginUtils的指针
    std::shared_ptr<PluginUtils> pluginUtils;
    Status GetModuleList(ServerContext* context, const NullRequest* request, ModuleList* response) override;
    Status CallModuleInterface(ServerContext* context, const ModuleCallRequest* request, ModuleCallResponse* response) override;
    Status SyncModuleRunningData(ServerContext* context, const NullRequest* request,ServerWriter<ModuleRunningDataResponse>* writer) override;

};

// ModuleService的异步


class ServiceLocalRpc {
    public:
        std::unique_ptr<Server> server;
        bool *isRunning;
        // 一个用于存放PluginUtils的指针
        std::shared_ptr<PluginUtils> pluginUtils;
        void RunServer();
        void StopServer();
};


#endif // SERVICE_LOCAL_RPC_H