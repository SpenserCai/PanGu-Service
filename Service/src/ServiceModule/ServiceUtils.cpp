/*
 * @Author: 饕餮
 * @Date: 2022-05-02 15:07:31
 * @version: 
 * @LastEditors: SpenserCai
 * @LastEditTime: 2023-02-08 15:41:18
 * @Description: file content
 */
#include <ServiceModule/ServiceUtils.h>




bool ServiceUtils::isDebugger() {
    // 多种方法检测当前程序是否被调试
    bool isDebugger = false;
    // 1. 检测PE文件是否被修改
    isDebugger = IsDebuggerPresent();
    // 2. 检测进程是否被调试
    //if (!isDebugger) {
    //    HANDLE hProcess = GetCurrentProcess();
    //    DWORD dwProcessId;
    //    dwProcessId = GetProcessId(hProcess);
    //    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    //    PROCESSENTRY32 pe32;
    //    pe32.dwSize = sizeof(PROCESSENTRY32);
    //    BOOL bRet = Process32First(hSnapshot, &pe32);
    //    while (bRet) {
    //        if (dwProcessId == pe32.th32ProcessID) {
    //            isDebugger = true;
    //            break;
    //        }
    //        bRet = Process32Next(hSnapshot, &pe32);
    //    }
    //    CloseHandle(hSnapshot);
    //}
    return isDebugger;
}

Json::Value ServiceUtils::getHardwareInfo()
{
    // 用一个列表来存储硬件信息
    Json::Value hardwareInfo;
    // 获取主板型号
    std::string motherboard = readRegistry("HARDWARE\\DESCRIPTION\\System\\BIOS", "BaseBoardManufacturer");  
    hardwareInfo["motherboard"] = motherboard;
    std::cout << hardwareInfo << std::endl;
    return hardwareInfo;
    
    
}

bool ServiceUtils::isVirtualMachine() {
    // 多种方法判断当前是否在虚拟机中
    bool isVM = false;
    // TODO:通过获取硬件信息判断是否在VMWare、Visor、VirtualBox中

    // 通过判断进程是否存在来判断是否在虚拟机中
    if (!isVM) {
        LPCSTR vmProcess[] = {
            "vboxservice.exe",
            "vboxtray.exe",
            "vmtoolsd.exe",
            "vmwaretray.exe",
            "vmwareuser",
            "vmsrvc.exe",
            "vmusrvc.exe",
            "prl_cc.exe",
            "prl_tools.exe",
            "xenservice.exe"
        };
        for (int i = 0; i < sizeof(vmProcess) / sizeof(vmProcess[0]); i++) {
            if (isProcessExist(vmProcess[i])) {
                isVM = true;
                break;
            }
        }
    }

    // 通过判断Windows目录中是否存在一下文件来判断是否在虚拟机中
    if (!isVM) {
        std::string vmFile[] = {
            "\\system32\\drivers\\VBoxMouse.sys",
            "\\system32\\drivers\\VBoxGuest.sys",
            "\\system32\\drivers\\VBoxSF.sys",
            "\\system32\\drivers\\VBoxVideo.sys",
            "\\system32\\vboxdisp.dll",
            "\\system32\\vboxhook.dll",
            "\\system32\\vboxmrxnp.dll",
            "\\system32\\vboxogl.dll",
            "\\system32\\vboxoglarrayspu.dll",
            "\\system32\\vboxoglcrutil.dll",
            "\\system32\\vboxoglerrorspu.dll",
            "\\system32\\vboxoglfeedbackspu.dll",
            "\\system32\\vboxoglpackspu.dll",
            "\\system32\\vboxoglpassthroughspu.dll",
            "\\system32\\vboxservice.exe",
            "\\system32\\vboxtray.exe",
            "\\system32\\VBoxControl.exe",
            "\\system32\\drivers\\vmmouse.sys",
            "\\system32\\drivers\\vmhgfs.sys"
        };
        // 获取Windows目录
        char windowsPath[MAX_PATH];
        GetWindowsDirectory(windowsPath, MAX_PATH);
        for (int i = 0; i < sizeof(vmFile) / sizeof(vmFile[0]); i++) {
            std::string filePath = windowsPath + vmFile[i];
            if (isFileExist(filePath)) {
                isVM = true;
                break;
            }
        }
    }

    return isVM;
    
}

void ServiceUtils::antiHacker() {
    // 反调试
    if (isDebugger()) {
        Json::Value debugAttackLog;
        serviceProtectBlueScreen();
    }
    // 虚拟机检测
    if (isVirtualMachine()) {
        Json::Value vmAttacklog;
        //reportLog("pangu-service","attack","Run in Virtual Machine",vmAttacklog);
        // serviceProtectBlueScreen();
    }
}


