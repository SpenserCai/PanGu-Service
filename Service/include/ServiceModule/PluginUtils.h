/*
 * @Author: 饕餮
 * @Date: 2022-05-08 20:30:13
 * @version: 
 * @LastEditors: 饕餮
 * @LastEditTime: 2022-06-09 16:48:40
 * @Description: file content
 */
#ifndef PLUGIN_UTILS_H
#define PLUGIN_UTILS_H

#include <Windows.h>
#include <string>
#include <json/json.h>
#include <queue>
#include "plugin_base.h"
#include <ServiceModule/ServiceUtils.h>



class PluginUtils {
    public:
        PluginUtils();
        ~PluginUtils();
        enum PLUGIN_RESULT {
            PLUGIN_SUCCESS,
            PLUGIN_INIT_FAILED, // 初始化插件失败
            PLUGIN_NOT_REGISTERED, // 插件未注册
            PLUGIN_NOT_EXIST, // 插件不存在
            PLUGIN_INVALID, // 插件无效 
        };
        // 声明一个插件结构体
        typedef struct {
            PluginBase *plugin;
            HMODULE pluginModule;
        } PluginObject, *PluginObjectPtr;
        std::string pluginDir;
        // 获取插件路径列表
        std::vector<std::string> GetPluginPathList();
        // 注册插件函数
        PLUGIN_RESULT RegisterPlugin(std::string pluginPath);
        // 反注册插件函数
        PLUGIN_RESULT UnRegisterPlugin(std::string pluginName);
        // 验证插件合法性
        PLUGIN_RESULT VerifyPlugin(std::string pluginPath);
        // 更新插件
        PLUGIN_RESULT UpdatePlugin(std::string pluginPath);
        // 通过插件文件路径获取插件名
        std::string GetPluginNameWithFilePath(std::string pluginPath);
        // 判断插件是否在列表中
        bool IsPluginInList(std::string pluginName);
        // 获取插件列表
        Json::Value GetPluginList();
        // 插件操作器
        PluginBase* GetPlugin(std::string pluginName);
        // 声明一个先进先出的RunningData队列
        std::queue<Json::Value> RunningDataQueue;
        // 声明一个互斥锁
        std::mutex RunningDataQueueMutex;
        // 判断插件是否加载完成
        bool IsPluginLoaded;
    private:
        // 插件列表
        map<std::string, PluginObjectPtr> plugin_list;
        void PluginReportLog(Json::Value& data);
        void PluginSendRunnginData(Json::Value& data);
        void SetAllPluginBaseCallBackFunc(PluginBase* plugin);
        
};


#endif