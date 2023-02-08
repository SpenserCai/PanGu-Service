/*
 * @Author: 饕餮
 * @Date: 2022-05-06 11:28:06
 * @version: 
 * @LastEditors: 饕餮
 * @LastEditTime: 2022-08-18 16:29:15
 * @Description: 工具方法
 */
#include "ServiceModule/ServiceUtils.h"

std::string ServiceUtils::getUserName() {
    //获取当前用户名
    std::string userName = "";
    char userNameBuf[256] = {0};
    DWORD userNameLen = 256;
    if (GetUserNameA(userNameBuf, &userNameLen)) {
        userName = userNameBuf;
    }
    return userName;

}

std::string ServiceUtils::getCurrentTime(const std::string& format) {
    // 获取当前时间
    std::string currentTime = "";
    time_t t = time(0);
    char timeBuf[64] = {0};
    if (strftime(timeBuf, sizeof(timeBuf), format.c_str(), localtime(&t))) {
        currentTime = timeBuf;
    }
    currentTime = timeBuf;
    return currentTime;
}

std::string ServiceUtils::readRegistry(const std::string& key, const std::string& valueName) {
    //读取注册表
    std::string value = "";
    HKEY hKey = NULL;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD dwType = REG_SZ;
        char valueBuf[256] = {0};
        DWORD valueLen = 256;
        if (RegQueryValueEx(hKey, valueName.c_str(), NULL, &dwType, (LPBYTE)valueBuf, &valueLen) == ERROR_SUCCESS) {
            value = valueBuf;
        }
        RegCloseKey(hKey);
    }
    return value;
}

bool ServiceUtils::isProcessExist(const LPCSTR processName)
{
    // 判断进程是否存在
    bool isExist = false;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    BOOL bRet = Process32First(hSnapshot, &pe32);
    while (bRet) {
        if (strcmp(pe32.szExeFile, processName) == 0) {
            isExist = true;
            break;
        }
        bRet = Process32Next(hSnapshot, &pe32);
    }
    CloseHandle(hSnapshot);
    return isExist;
}

bool ServiceUtils::isFileExist(const std::string& filePath)
{
    // 判断文件是否存在
    bool isExist = false;
    if (FILE* file = fopen(filePath.c_str(), "r")) {
        isExist = true;
        fclose(file);
    }
    return isExist;
}

DWORD ServiceUtils::getProcessIdWithName(const std::string& processName)
{
    // 获取进程id
    DWORD processId = 0;
    // 加载进程列表
    std::vector<PROCESSENTRY32> processList;
    processList = getProcessList();
    // 查找进程
    for (auto& process : processList) {
        if (strcmp(process.szExeFile, processName.c_str()) == 0) {
            processId = process.th32ProcessID;
            break;
        }
    }
    return processId;
}

std::vector<PROCESSENTRY32> ServiceUtils::getProcessList()
{
    // 获取进程列表
    std::vector<PROCESSENTRY32> processList;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    BOOL bRet = Process32First(hSnapshot, &pe32);
    while (bRet) {
        processList.push_back(pe32);
        bRet = Process32Next(hSnapshot, &pe32);
    }
    CloseHandle(hSnapshot);
    return processList;
}

bool ServiceUtils::isJson(const std::string& jsonStr)
{
    // 判断是否是json
    bool isJson = false;
    try {
        // 转成json对象
        Json::Value tmpJson;
        Json::Reader reader;
        if (reader.parse(jsonStr, tmpJson)) {
            isJson = true;
        }
        else {
            isJson = false;
        }
    } catch (std::exception& e) {
        isJson = false;
    }
    return isJson;
}

std::string ServiceUtils::getCurrentProcessPath()
{
    // 获取当前进程目录
    char path[MAX_PATH] = {0};
    GetModuleFileName(NULL, path, MAX_PATH);
    std::string processPath = path;
    // 仅保留目录部分
    int pos = processPath.find_last_of("\\");
    processPath = processPath.substr(0, pos);
    return processPath;
}

bool ServiceUtils::copyFile(const std::string& srcFile, const std::string& destFile)
{
    // 拷贝文件
    bool isCopy = false;
    if (isFileExist(srcFile)) {
        if (CopyFile(srcFile.c_str(), destFile.c_str(), FALSE)) {
            isCopy = true;
        }
    }
    return isCopy;
}

std::vector<std::string> ServiceUtils::getFileListByRegex(const std::string& dirPath, const std::string& regex)
{
    // 获取指定目录下符合格式的文件名
    std::vector<std::string> fileList;
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile((dirPath + regex).c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string filename = fd.cFileName;
                fileList.push_back(dirPath + filename);
            }
        } while (FindNextFile(hFind, &fd));
        FindClose(hFind);
    }
    return fileList;
}

wchar_t* ServiceUtils::char2wchar(const char* cchar)
{
    wchar_t *m_wchar;
    int len = MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), NULL, 0);
    m_wchar = new wchar_t[len + 1];
    MultiByteToWideChar(CP_ACP, 0, cchar, strlen(cchar), m_wchar, len);
    m_wchar[len] = '\0';
    return m_wchar;
}

std::string ServiceUtils::wchar2char(const wchar_t* wchar)
{
    char *m_char;
    int len = WideCharToMultiByte(CP_ACP, 0, wchar, -1, NULL, 0, NULL, NULL);
    m_char = new char[len + 1];
    WideCharToMultiByte(CP_ACP, 0, wchar, -1, m_char, len, NULL, NULL);
    m_char[len] = '\0';
    return m_char;
}

HANDLE ServiceUtils::GetProcessUserToken(HANDLE processHandle)
{
    // 通过进程句柄获取进程用户令牌
    HANDLE hToken = NULL;
    if (OpenProcessToken(processHandle, TOKEN_QUERY, &hToken))
    {
        return hToken;
    }
    return NULL;
}

HANDLE ServiceUtils::GetProcessHandleByName(const char* processName)
{
    // 通过进程名获取HANDLE
    HANDLE hProcess = NULL;
    PROCESSENTRY32 pe32;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnapshot, &pe32))
    {
        CloseHandle(hSnapshot);
        return NULL;
    }
    do
    {
        if (strcmp(pe32.szExeFile, processName) == 0)
        {
            hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));
    CloseHandle(hSnapshot);
    return hProcess;
}