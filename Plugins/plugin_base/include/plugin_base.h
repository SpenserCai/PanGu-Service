/*
 * @Author: 饕餮
 * @Date: 2022-05-17 20:52:11
 * @version: 
 * @LastEditors: 饕餮
 * @LastEditTime: 2022-05-27 01:20:11
 * @Description: file content
 */
#ifndef PLUGIN_BASE_H
#define PLUGIN_BASE_H

#pragma warning(disable:4251)

#if !defined(_PLUGIN_BASE_LIB_)
#define PLUGINBASEAPI __declspec(dllimport)
#else
#define PLUGINBASEAPI __declspec(dllexport)
#endif

#include <Windows.h>
#include <string>
#include <iostream>
#include <functional>
#include <json/json.h>
#include <functional>
#include <chrono>
#include <map>
using namespace std;

// 插件基类
class PLUGINBASEAPI PluginBase
{
    public:
        // 构建函数
        PluginBase();
        // 获取插件信息
        Json::Value GetPluginInfo();
        // 插件函数(支持子类)
        typedef Json::Value(*PluginFunc)(Json::Value&);
        // 插件函数列表
        map<string, std::function<Json::Value(Json::Value&)>> PluginFuncList;
        // 调用插件函数
        Json::Value CallPluginFunc(const string& func_name, Json::Value& args);
        // 设置插件函数列表
        void SetPluginFuncList(const string& func_name, std::function<Json::Value(Json::Value&)> func);
        // 设置回调函数列表
        void SetCallBackFuncList(const string& func_name, std::function<void(Json::Value&)> func);
        // 插件释放
        virtual void DestroyPlugin() = 0;
        // 检测和安装依赖
        virtual void CheckAndInstallDependency() = 0;

    protected:
        // 设置插件名
        void SetPluginName(const string& plugin_name);
        // 设置插件版本
        void SetPluginVersion(const string& plugin_version);
        // 设置插件作者
        void SetPluginAuthor(const string& plugin_author);
        // 设置插件简介
        void SetPluginDesc(const string& plugin_desc);
        // 设置插件类型
        void SetPluginType(const string& plugin_type);
        // 调用回调函数
        void CallBackFunc(const string& func_name, Json::Value& args);
        // 插件上报日志
        void ReportLog(const std::string& type, const std::string& title, Json::Value& content);
        // 用于通过服务回传给electron端的运行数据，包括各种状态等
        void SendRunningData(const std::string& type, Json::Value& content);
    private:
        // 插件信息
        Json::Value pluginInfo;
        // 用于存放回调函数
        map<string, std::function<void(Json::Value&)>> CallBackFuncList;
        
        
        
        
};

#endif