/*
 * @Author: 饕餮
 * @Date: 2022-05-08 20:37:25
 * @version: 
 * @LastEditors: SpenserCai
 * @LastEditTime: 2023-02-08 15:18:51
 * @Description: file content
 */

#include <ServiceModule/PluginUtils.h>


PluginUtils::PluginUtils()
{
    this->IsPluginLoaded = false;
    // 设置插件目录
    pluginDir = ServiceUtils::getCurrentProcessPath() + "\\Plugins\\";
    auto pluginPathList = GetPluginPathList();
    for (auto pluginPath : pluginPathList) {
        RegisterPlugin(pluginPath);
    }
    this->IsPluginLoaded = true;
}

//void PluginUtils::PluginReportLog(Json::Value& data)
//{
//    ServiceUtils::reportLog("pangu-service-plugin", data["type"].asString(),data["title"].asString(),data["content"]);
//}

void PluginUtils::PluginSendRunnginData(Json::Value& data)
{
    std::lock_guard<std::mutex> lock(this->RunningDataQueueMutex);
    RunningDataQueue.push(data);
}

// 用于设置插件的基础回调函数
void PluginUtils::SetAllPluginBaseCallBackFunc(PluginBase* plugin)
{
    // plugin->SetCallBackFuncList("ReportLog", std::bind(&PluginUtils::PluginReportLog, this, std::placeholders::_1));
    plugin->SetCallBackFuncList("SendRunningData", std::bind(&PluginUtils::PluginSendRunnginData, this, std::placeholders::_1));
}

PluginUtils::~PluginUtils()
{
    // 对象释放时卸载所有插件
    for (auto it = plugin_list.begin(); it != plugin_list.end(); it++)
    {
        it->second->plugin->DestroyPlugin();
        FreeLibrary(it->second->pluginModule);
    }
    plugin_list.clear();
    Json::Value logContent;
    logContent["data"] = "destory pluginutil";
    //ServiceUtils::reportLog("pangu-service-plugin","info","Plugin",logContent);
}

bool PluginUtils::IsPluginInList(std::string pluginName)
{
    if (plugin_list.find(pluginName) == plugin_list.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}

PluginUtils::PLUGIN_RESULT PluginUtils::UpdatePlugin(std::string pluginPath)
{
    // 如果插件存在于列表中择先卸载
    std::string pluginName = GetPluginNameWithFilePath(pluginPath);
    if (IsPluginInList(pluginName))
    {
        UnRegisterPlugin(pluginName);
    }
    // 将pluginPath的文件复制到插件目录
    ServiceUtils::copyFile(pluginPath, pluginDir + pluginName + ".dll");
    PLUGIN_RESULT registerResult;
    // 注册插件
    registerResult = RegisterPlugin(pluginDir + pluginName + ".dll");
    return registerResult;
}

std::string PluginUtils::GetPluginNameWithFilePath(std::string pluginPath)
{
    std::string pluginName = pluginPath.substr(pluginPath.find_last_of("\\") + 1);
    pluginName = pluginName.substr(0, pluginName.find_last_of("."));
    return pluginName;
}

PluginUtils::PLUGIN_RESULT PluginUtils::RegisterPlugin(std::string pluginPath)
{
    
    //std::cout<<pluginPath<<std::endl;
    std::string pluginName = GetPluginNameWithFilePath(pluginPath);
    HMODULE pluginModule = LoadLibrary(pluginPath.c_str());
    if (pluginModule == NULL)
    {
        std::cout<<"load plugin error"<<std::endl;
        return PLUGIN_NOT_EXIST;
    }
    if (ServiceUtils::verifySignature(pluginPath) == false)
    {
        Json::Value logContent;
        logContent["plugin_name"] = pluginName;
        logContent["data"] = "plugin signature verify failed";
        // ServiceUtils::reportLog("pangu-service-plugin","error","Plugin Security",logContent);
        return PLUGIN_INVALID;
    }
    //std::cout<<"load plugin module success:"<<pluginName<<std::endl;
    typedef bool (*GetPluginObjectFunc)(void **);
    GetPluginObjectFunc GetPluginObject = (GetPluginObjectFunc)GetProcAddress(pluginModule, "CreatePlugin");
    if (GetPluginObject == NULL)
    {
        Json::Value logContent;
        logContent["plugin_name"] = pluginName;
        // ServiceUtils::reportLog("pangu-service-plugin","error","Plugin Invalid",logContent);
        FreeLibrary(pluginModule);
        return PLUGIN_INVALID;
    }
    //std::cout<<"get plugin object success:"<<pluginName<<std::endl;
    PluginBase* plugin = NULL;
    if (!GetPluginObject(reinterpret_cast<void**>(&plugin)))
    {
        return PLUGIN_INIT_FAILED;
    }
    //std::cout<<"init plugin success:"<<pluginName<<std::endl;
    // 如果插件已经注册，则反注册后重新注册
    if (IsPluginInList(pluginName))
    {
        UnRegisterPlugin(pluginName);
    }
    SetAllPluginBaseCallBackFunc(plugin);
    // 使用结构体存储插件
    PluginObject *pluginObject = new PluginObject;
    pluginObject->plugin = plugin;
    pluginObject->pluginModule = pluginModule;
    plugin_list[pluginName] = pluginObject;
    return PLUGIN_SUCCESS;
}

PluginUtils::PLUGIN_RESULT PluginUtils::UnRegisterPlugin(std::string pluginName)
{
    if (plugin_list.find(pluginName) == plugin_list.end())
    {
        return PLUGIN_NOT_EXIST;
    }
    plugin_list[pluginName]->plugin->DestroyPlugin();
    FreeLibrary(plugin_list[pluginName]->pluginModule);
    plugin_list.erase(pluginName);
    return PLUGIN_SUCCESS;
}

PluginBase* PluginUtils::GetPlugin(std::string pluginName)
{
    if (!IsPluginInList(pluginName))
    {
        return NULL;
    }
    return plugin_list[pluginName]->plugin;
}

Json::Value PluginUtils::GetPluginList()
{
    Json::Value pluginList;
    for (auto it = plugin_list.begin(); it != plugin_list.end(); it++)
    {
        auto interface = it->second->plugin->PluginFuncList;
        // 声明一个Json的字符串数组
        Json::Value interfaceList;
        for (auto it = interface.begin(); it != interface.end(); it++)
        {
            interfaceList.append(it->first);
        }
        pluginList[it->first]["info"] = it->second->plugin->GetPluginInfo();
        pluginList[it->first]["interface"] = interfaceList;
    }
    return pluginList;
}

std::vector<std::string> PluginUtils::GetPluginPathList()
{
    // 获取插件目录下的所有plugin_*.dll文件路径
    std::vector<std::string> pluginFileList = ServiceUtils::getFileListByRegex(pluginDir, "plugin_*.dll");
    return pluginFileList;
}
