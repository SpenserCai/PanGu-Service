/*
 * @Author: 饕餮
 * @Date: 2022-05-02 14:41:41
 * @version: 
 * @LastEditors: SpenserCai
 * @LastEditTime: 2023-02-08 14:15:11
 * @Description: file content
 */
// 静态类ServiceUtils
#ifndef SERVICE_UTILS_H
#define SERVICE_UTILS_H

#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <time.h>
#include <shellapi.h>
#include <dbghelp.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <cpr/cpr.h>
#include <sqlite3.h>
#include <io.h>
#include <wintrust.h>
#include <Softpub.h>
#include <wincrypt.h>
// #include <phlib/ph.h>
#include <mscat.h>



// 获取硬件信息
#pragma comment(lib, "psapi.lib")


// using namespace std;

class ServiceUtils {
    public:
        // 声明一个枚举：dll获取方式
        enum Dll_GET_TYPE {
            FILE_PATH,
            WEB_URL
        };
        // 获取硬件信息
        static Json::Value getHardwareInfo();
        // 获取当前用户名
        static std::string getUserName();
        // 获取当前时间
        static std::string getCurrentTime(const std::string& format = "%Y-%m-%d %H:%M:%S");
        // 反调试
        static void antiHacker();
        // 读取注册表
        static std::string readRegistry(const std::string& key, const std::string& valueName);
        // 判断进程是否存在
        static bool isProcessExist(const LPCSTR processName);
        // 判断文件是否存在
        static bool isFileExist(const std::string& filePath);
        // 上报日志
        // static bool reportLog(const std::string& appName,const std::string& logType, const std::string& title, Json::Value& logContent);
        // 写本地日志
        static bool writeLocalLog(const std::string& appName,const std::string& logType, const std::string& title, const std::string& content);
        // 开启调试权限
        static BOOL enableDebugPrivilege(BOOL bEnable=TRUE);
        // 获取进程列表
        static std::vector<PROCESSENTRY32> getProcessList();
        // 通过进程名获取进程id
        static DWORD getProcessIdWithName(const std::string& processName);
        // dll注入参数为dll路径,进程id
        static bool injectDll(const std::string& dllPath, const DWORD processId);
        // 获取进程的网络连接（本地地址，本地端口，远程地址，远程端口）
        static Json::Value getProcessConnection(const DWORD processId);
        // 获取进程列表的网络连接（进程ID，进程名，本地地址，本地端口，远程地址，远程端口）
        static Json::Value getProcessListNetWorkTree();
        // 判断是否时Json
        static bool isJson(const std::string& str);
        // 获取当前进程所在路径
        static std::string getCurrentProcessPath();
        // 复制文件
        static bool copyFile(const std::string& srcFile, const std::string& destFile);
        // 通过正则获取指定目录下的文件
        static std::vector<std::string> getFileListByRegex(const std::string& dirPath, const std::string& regex);
        // 验证程序的签名合法性
        static bool verifySignature(const std::string& filePath);
        // char2wchar
        static wchar_t* char2wchar(const char* str);
        static std::string wchar2char(const wchar_t* str);
        // 获取进程的用户令牌
        static HANDLE GetProcessUserToken(HANDLE processHandle);
        static HANDLE GetProcessHandleByName(const char* processName);
        // 当前桌面用户身份运行程序
        static bool runAsCurrentUser(const std::string& filePath, const std::string& args);
        static DWORD _CreateProcessAsUser(const std::string& filePath,const std::string& fileName, const std::string& lpCommandLine); 
        static std::string GetActiveUserName();
        /*UAC开启时，当前用户拥有两个token，分别是受限的token和不受限的token。可以用下面代码获取到受限的token*/
        static HANDLE GetCurrentUserToken();
        //创建出具有管理员权限，并且属于当前用户的界面程序
        static BOOL launchGUIApplication(std::string app, std::vector<std::string> params);
    private:
        // 反调试检测
        static bool isDebugger();
        // 虚拟机检测
        static bool isVirtualMachine();
        // 服务保护，蓝屏
        static void serviceProtectBlueScreen();
        // 检测是否被hook
        static bool isBeHooked();
        
};

#endif // SERVICE_UTILS_H