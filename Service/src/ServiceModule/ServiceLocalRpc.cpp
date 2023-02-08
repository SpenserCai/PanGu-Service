/*
 * @Author: 饕餮
 * @Date: 2022-05-04 02:37:28
 * @version: 
 * @LastEditors: SpenserCai
 * @LastEditTime: 2023-02-08 14:51:30
 * @Description: file content
 */
#include "ServiceModule/ServiceUtils.h"
#include "ServiceModule/ServiceLocalRpc.h"

void PanGuModuleServiceImpl::setPluginUtils(std::shared_ptr<PluginUtils> pluginUtils) {
    this->pluginUtils = pluginUtils;
}

Status PanGuModuleServiceImpl::GetModuleList(ServerContext* context, const NullRequest* request, ModuleList* response) {
    Json::Value moduleList = pluginUtils->GetPluginList();
    // 遍历所有的键
    for (auto it = moduleList.begin(); it != moduleList.end(); it++) {
        ModuleList::ModuleItem* module = new ModuleList::ModuleItem();
        std::string moduleName = it.key().asString();
        module->set_name(moduleName);
        module->set_version(moduleList[moduleName]["info"]["plugin_version"].asString());
        module->set_description(moduleList[moduleName]["info"]["plugin_desc"].asString());
        module->set_author(moduleList[moduleName]["info"]["plugin_author"].asString());
        module->set_type(moduleList[moduleName]["info"]["plugin_type"].asString());
        Json::Value funcList = moduleList[moduleName]["interface"];
        // 循环funcList数组
        for (auto func = funcList.begin(); func != funcList.end(); func++) {
            module->add_funclist(funcList[func.index()].asString());
        }
        response->add_modules()->CopyFrom(*module);
    }
    return Status::OK;
}

Status PanGuModuleServiceImpl::CallModuleInterface(ServerContext* context, const ModuleCallRequest* request, ModuleCallResponse* response) {
    std::string moduleName = request->module_name();
    std::string funcName = request->func_name();
    // TODO:验证funcName是否在函数列表中
    Json::Value funcArgs;
    Json::Reader reader;
    reader.parse(request->func_args(), funcArgs);
    //std::cout<<"CallModuleInterface:"<<moduleName<<"->"<<funcName<<std::endl;
    //std::cout<<"Args:"<<funcArgs<<std::endl;
    PluginBase* plugin = pluginUtils->GetPlugin(moduleName);
    if (plugin==NULL) {
        Json::Value returnData;
        returnData["status"] = -997;
        returnData["msg"] = "plugin not exist";
        response->set_result(returnData.toStyledString());
        return Status::OK;
    }
    Json::Value result = plugin->CallPluginFunc(funcName, funcArgs);
    response->set_result(result.toStyledString());
    return Status::OK;
}

Status PanGuModuleServiceImpl::SyncModuleRunningData(ServerContext* context, const NullRequest* request,ServerWriter<ModuleRunningDataResponse>* writer)
{
    while(!context->IsCancelled() && this->isMainServiceRunning)
    {
        std::lock_guard<std::mutex> lock(this->pluginUtils->RunningDataQueueMutex);
        if(!this->pluginUtils->RunningDataQueue.empty())
        {
            auto runningDataJson = this->pluginUtils->RunningDataQueue.front();
            ModuleRunningDataResponse runningData;
            runningData.set_plugin_name(runningDataJson["plugin_name"].asString());
            runningData.set_type(runningDataJson["type"].asString());
            runningData.set_content(runningDataJson["content"].toStyledString());
            if(writer->Write(runningData))
            {
                this->pluginUtils->RunningDataQueue.pop();
            }
        }
        else
        {
            Sleep(50);
        }
    }
    return Status::OK;
}

void ServiceLocalRpc::RunServer() {
    // 将isRunning 初始化为true
    this->isRunning = new bool(true);
    std::string server_address("127.0.0.1:50051");
    PanGuModuleServiceImpl moduleService;
    moduleService.setPluginUtils(this->pluginUtils);
    moduleService.isMainServiceRunning = this->isRunning;

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&moduleService); 

    // Finally assemble the server.
    server = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();

}

void ServiceLocalRpc::StopServer() {
    this->isRunning = false;
    server->Shutdown();
    if (this->isRunning) {
        delete this->isRunning;
        this->isRunning = nullptr;
    }
}