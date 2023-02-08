/*
 * @Author: 饕餮
 * @Date: 2022-05-17 20:51:23
 * @version: 
 * @LastEditors: 饕餮
 * @LastEditTime: 2022-05-27 01:30:20
 * @Description: file content
 */
#include "plugin_base.h"

void PluginBase::SetPluginName(const string& plugin_name)
{
    pluginInfo["plugin_name"] = plugin_name;
}

void PluginBase::SetPluginVersion(const string& plugin_version)
{
    pluginInfo["plugin_version"] = plugin_version;
}

void PluginBase::SetPluginAuthor(const string& plugin_author)
{
    pluginInfo["plugin_author"] = plugin_author;
}

void PluginBase::SetPluginDesc(const string& plugin_desc)
{
    pluginInfo["plugin_desc"] = plugin_desc;
}

void PluginBase::SetPluginType(const string& plugin_type)
{
    pluginInfo["plugin_type"] = plugin_type;
}

PluginBase::PluginBase()
{
    
}

Json::Value PluginBase::GetPluginInfo()
{
    return pluginInfo;
}

Json::Value PluginBase::CallPluginFunc(const string& func_name, Json::Value& args)
{
    Json::Value returnData;
    try
    {
        if(PluginFuncList.find(func_name) == PluginFuncList.end())
        {
            returnData["status"] = -998;
            returnData["msg"] = "plugin func not found";
            return returnData;
        }
        return PluginFuncList[func_name](args);
    }
    catch(const std::exception& e)
    {
        returnData["status"] = -999;
        returnData["msg"] = e.what();
        return returnData;
    }
    
}

void PluginBase::SetPluginFuncList(const string& func_name, std::function<Json::Value(Json::Value&)> func)
{
    PluginFuncList[func_name] = func;
}

void PluginBase::CallBackFunc(const string& func_name, Json::Value& args)
{
    if(CallBackFuncList.find(func_name) == CallBackFuncList.end())
    {
        return;
    }
    CallBackFuncList[func_name](args);
}

void PluginBase::SetCallBackFuncList(const string& func_name, std::function<void(Json::Value&)> func)
{
    CallBackFuncList[func_name] = func;
}

void PluginBase::ReportLog(const std::string& type, const std::string& title, Json::Value& content)
{
    content["plugin_name"] = pluginInfo["plugin_name"];
    Json::Value logData;
    logData["type"] = type;
    logData["title"] = title;
    logData["content"] = content;
    CallBackFunc("ReportLog", logData);
}

void PluginBase::SendRunningData(const std::string& type,Json::Value& content)
{
    // 获取时间戳，使用c++11的chrono
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();

    Json::Value runningData;
    runningData["plugin_name"] = pluginInfo["plugin_name"];
    runningData["type"] = type;
    runningData["content"] = content;
    runningData["content"]["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(value).count();
    CallBackFunc("SendRunningData", runningData);
}