/*
 * @Author: 饕餮
 * @Date: 2022-05-07 22:39:57
 * @version: 
 * @LastEditors: 饕餮
 * @LastEditTime: 2022-08-18 16:29:27
 * @Description: file content
 */

#include <ServiceModule/ServiceUtils.h>

Json::Value ServiceUtils::getProcessConnection(const DWORD processId)
{
    Json::Value processConnection;
    ULONG ulSize = sizeof(MIB_TCPTABLE2);
    PMIB_TCPTABLE2 pTcpTable = (PMIB_TCPTABLE2)malloc(ulSize);
    if (pTcpTable == nullptr) {
        std::cout << "malloc failed" << std::endl;
        return processConnection;
    }
    if (GetTcpTable2(pTcpTable, &ulSize, TRUE) == ERROR_INSUFFICIENT_BUFFER) {
        free(pTcpTable);
        pTcpTable = (PMIB_TCPTABLE2)malloc(ulSize);
        if (pTcpTable == nullptr) {
            std::cout << "malloc failed" << std::endl;
            return processConnection;
        }
    }
    if (GetTcpTable2(pTcpTable, &ulSize, TRUE) == NO_ERROR) {
        for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
            if (pTcpTable->table[i].dwOwningPid == processId) {
                processConnection["local_addr"] = inet_ntoa(*(in_addr*)&pTcpTable->table[i].dwLocalAddr);
                processConnection["remote_addr"] = inet_ntoa(*(in_addr*)&pTcpTable->table[i].dwRemoteAddr);
                processConnection["local_port"] = (int)pTcpTable->table[i].dwLocalPort;
                processConnection["remote_port"] = (int)pTcpTable->table[i].dwLocalPort;
                processConnection["state"] = (double)pTcpTable->table[i].dwState;
                processConnection["process_id"] = (int)pTcpTable->table[i].dwOwningPid;
            }
        }
    }
    free(pTcpTable);
    return processConnection;
    
}

Json::Value ServiceUtils::getProcessListNetWorkTree()
{
    Json::Value networkTree;
    // 获取进程列表
    std::vector<PROCESSENTRY32> processList = getProcessList();
    // 获取网络连接列表
    for(auto &process : processList) {
        // 获取进程id
        DWORD processId = process.th32ProcessID;
        // 获取进程名
        std::string processName = process.szExeFile;
        // 获取进程的网络连接
        Json::Value processConnection = getProcessConnection(processId);
        if(processConnection.empty()) {
            continue;
        }
        networkTree[processName] = processConnection;
    }
    return networkTree;
}