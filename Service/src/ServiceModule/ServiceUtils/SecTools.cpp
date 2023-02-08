/*
 * @Author: 饕餮
 * @Date: 2022-05-06 13:19:31
 * @version: 
 * @LastEditors: 饕餮
 * @LastEditTime: 2022-08-19 16:58:54
 * @Description: file content
 */
#include "ServiceModule/ServiceUtils.h"
#include <WtsApi32.h>
#include <Userenv.h>
#include <tchar.h>
#pragma comment(lib, "WtsApi32.lib")
#pragma comment(lib, "Userenv.lib")

BOOL ServiceUtils::enableDebugPrivilege(BOOL bEnable) {
    BOOL fOK = FALSE;
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
        tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
        AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL);
        fOK = (GetLastError() == ERROR_SUCCESS);
        CloseHandle(hToken);
    }
    return fOK;

}

void ServiceUtils::serviceProtectBlueScreen() {
    // 通过调用蓝屏报错
    HMODULE ntdll = LoadLibrary("ntdll.dll");//加载ntdll
    FARPROC RtlAdjPriv = GetProcAddress(ntdll, "RtlAdjustPrivilege");//获取提权函数
    FARPROC ZwRaiseHardErr = GetProcAddress(ntdll, "ZwRaiseHardError");//获取蓝屏函数
    unsigned char ErrKill;
    long unsigned int HDErr;
    ((void(*)(DWORD, DWORD, BOOLEAN, LPBYTE))RtlAdjPriv)(0x13, true, false, &ErrKill);//调用RtlAdjustPrivliege函数获取SeShutdownPrivilege权限
    ((void(*)(DWORD, DWORD, DWORD, DWORD, DWORD, LPDWORD))ZwRaiseHardErr)(0xc0000233, 0, 0, 0, 6, &HDErr);//使用ZwRaiseHardError制造蓝屏
}


bool ServiceUtils::injectDll(const std::string& dllPath, const DWORD processId) {
    // 提权
    if (!enableDebugPrivilege(TRUE)) {
        std::cout << "Enable Debug Privilege Failed" << std::endl;
        return false;
    }
    std::cout<<getProcessListNetWorkTree()<<std::endl;
    return true;
}

// 验证dll或者exe的签名合法性
bool ServiceUtils::verifySignature(const std::string& filePath) {
    GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_FILE_INFO fileInfo;
    WINTRUST_DATA wintrustData;
    HRESULT hResult;

    memset((void*)&fileInfo, 0, sizeof(WINTRUST_FILE_INFO));
    memset((void*)&wintrustData, 0, sizeof(WINTRUST_DATA));

    fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    fileInfo.pcwszFilePath = ServiceUtils::char2wchar(filePath.c_str());
    fileInfo.hFile = NULL;

    wintrustData.cbStruct = sizeof(WINTRUST_DATA);
    wintrustData.dwUIChoice = WTD_UI_NONE;
    wintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    wintrustData.dwUnionChoice = WTD_CHOICE_FILE;
    wintrustData.dwStateAction = WTD_STATEACTION_VERIFY;
    wintrustData.pFile = &fileInfo;
    // 验证数字签名合法性
    hResult = WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &wintrustData);
    if (hResult != S_OK) {
        std::cout << "WinVerifyTrust failed: " << hResult << std::endl;
        return false;
    }
    
    CRYPT_PROVIDER_DATA* pProvData = NULL;
    CRYPT_PROVIDER_CERT* pProvCert = NULL;
    CRYPT_PROVIDER_SGNR* pProvSgnr = NULL;
    
    pProvData = WTHelperProvDataFromStateData(wintrustData.hWVTStateData);
    if (pProvData == NULL) {
        std::cout << "WTHelperProvDataFromStateData failed" << std::endl;
        return false;
    }
    pProvSgnr = WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA)pProvData, 0, FALSE,0);
    if (pProvSgnr == NULL) {
        std::cout << "WTHelperGetProvSignerFromChain failed" << std::endl;
        return false;
    }
    pProvCert = WTHelperGetProvCertFromChain(pProvSgnr,0);
    
    if (pProvCert == NULL) {
        std::cout << "WTHelperGetProvCertFromChain failed" << std::endl;
        return false;
    }

    if (pProvCert->pCert == NULL) {
        std::cout << "pProvCert->pCert is NULL" << std::endl;
        return false;
    }
    
    // 读取完成的证书信息
    DWORD dwStrType;
    DWORD dwCount;
    LPTSTR szSubjectRDN = NULL;

    dwStrType = CERT_X500_NAME_STR;
    dwCount = CertGetNameString(pProvCert->pCert, CERT_NAME_RDN_TYPE, 0, &dwStrType, NULL, 0);
    if (dwCount == 0) {
        std::cout << "CertGetNameString failed" << std::endl;
        return false;
    }
    szSubjectRDN = (LPTSTR)malloc(dwCount * sizeof(TCHAR));
    CertGetNameString(pProvCert->pCert, CERT_NAME_RDN_TYPE, 0, &dwStrType, szSubjectRDN, dwCount);

    // 读取证书的序列号，用逗号分割szSubjectRDN
    std::string strSubjectRDN = szSubjectRDN;
    std::stringstream ss(strSubjectRDN);
    std::string item;
    std::vector<std::string> vec;
    while (std::getline(ss, item, ',')) {
        vec.push_back(item);
    }
    free(szSubjectRDN);
    szSubjectRDN = NULL;
    // 找到vec中存在SERIALNUMBER的元素，并获取等于符号后的值
    for (int i = 0; i < vec.size(); i++) {
        if (vec[i].find("SERIALNUMBER") != std::string::npos) {
            std::string strSerialNumber = vec[i].substr(vec[i].find("=") + 1);
            if (strSerialNumber == "91310106312514270C")
            {
                std::cout << "Verify Signature Success" << std::endl;
                return true;
            }
        }
    }
    std::cout << "Verify Signature Failed" << std::endl;
    return false;
}

// 以当前桌面用户省份运行程序
bool ServiceUtils::runAsCurrentUser(const std::string& filePath, const std::string& args) {
    HANDLE hProcess;
    hProcess = GetProcessHandleByName("explorer.exe");
    HANDLE hToken;
    hToken = GetProcessUserToken(hProcess);
    char tokenInfoBuf[256] = { 0 };
    DWORD dwReturnLength = 0;
    PTOKEN_USER pTokenInfo = (PTOKEN_USER)tokenInfoBuf;
    if (!GetTokenInformation(hToken, TokenUser, pTokenInfo, sizeof(tokenInfoBuf), &dwReturnLength))
    {
        CloseHandle(hToken);
        CloseHandle(hProcess);
        std::cout << "GetTokenInformation failed" << std::endl;
        return false;
    }
    std::cout << "pTokenInfo->User.Sid=" << pTokenInfo->User.Sid << std::endl;

    STARTUPINFO si = { 0 };
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = "WinSta0\\Default";

    LPVOID lpEnvironment = NULL;
    PROCESS_INFORMATION pi = { 0 };

    // 通过用户Token创建进程
    if (!CreateProcessAsUser(hToken, filePath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cout << "CreateProcessAsUser failed" << std::endl;
        return false;
    }
    std::cout << "CreateProcessAsUser success" << std::endl;
    CloseHandle(hToken);
    CloseHandle(hProcess);
    return true;
}

std::string ServiceUtils::GetActiveUserName()
{
    DWORD sessionId = WTSGetActiveConsoleSessionId();
    std::string userName;
    if (sessionId != 0xFFFFFFFF)
    {
        LPSTR userNameBuf = new char[MAX_PATH];
        DWORD userNameBufSize = UNLEN + 1;
        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, sessionId, WTSUserName, &userNameBuf, &userNameBufSize))
        {
            userName = userNameBuf;
        }
    }
    return userName;
}

DWORD ServiceUtils::_CreateProcessAsUser(const std::string& filePath, const std::string& fileName, const std::string& lpCommandLine)
{
    DWORD pId = 0;// result

    LUID luid; // local uniq id for process

    HANDLE TokenHandle = NULL;

    TOKEN_PRIVILEGES NewState = { 0 };
    TOKEN_PRIVILEGES PreviousState = { 0 };

    HANDLE phToken = NULL;
    HANDLE phNewToken = NULL;

    STARTUPINFO si = { 0 };
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = "WinSta0\\Default";// current user desktop

    LPVOID lpEnvironment = NULL;
    PROCESS_INFORMATION pi = { 0 };

    DWORD ReturnLength;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &TokenHandle))// read current process token
    {
        std::cout << "OpenProcessToken failed" << std::endl;
         return -1;
    }
    if (!LookupPrivilegeValue(NULL, "SeTcbPrivilege", &luid)) 
    {
        std::cout << "LookupPrivilegeValue failed" << std::endl;
        return -1;
    }

    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = luid;
    NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(TokenHandle, FALSE, &NewState, sizeof(TOKEN_PRIVILEGES), &PreviousState, &ReturnLength))// change proc privileges to user
    {
        std::cout << "AdjustTokenPrivileges failed" << std::endl;
        return -1;
    }

    DWORD sessionId = WTSGetActiveConsoleSessionId();

    // std::cout << "sessionId=" << sessionId << std::endl;

    if (sessionId == 0xFFFFFFFF) 
    {
        std::cout << "WTSGetActiveConsoleSessionId failed" << std::endl;
        return -1;
    }

    if (!WTSQueryUserToken(sessionId, &phToken))
    {
        std::cout << "WTSQueryUserToken failed" << std::endl;
        return -1;
    }

    if (!DuplicateTokenEx(phToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &phNewToken)) 
    {
        std::cout << "DuplicateTokenEx failed" << std::endl;
        return -1;
    }

    // if (!CreateEnvironmentBlock(&lpEnvironment, phToken, FALSE)) 
    // {
    //     std::cout << "CreateEnvironmentBlock failed" << std::endl;
    //     return -1;
    // }
    // std::cout << phNewToken << std::endl;
    std::string fullPath = filePath + "\\" + fileName;
    std::string cmdLine = "\"" + fullPath + "\" " + lpCommandLine;
    std::cout << fullPath << std::endl;

    if (!CreateProcessAsUser(phNewToken, fullPath.c_str(), (LPSTR)cmdLine.c_str(), NULL, NULL, FALSE, 0, NULL, filePath.c_str(), &si, &pi))// for hollowing CREATE_SUSPENDED | CREATE_NO_WINDOW 
    {
        std::cout << "CreateProcessAsUser failed:" << GetLastError() <<std::endl;
        return -1;
    }
    pId = pi.dwProcessId;

    AdjustTokenPrivileges(TokenHandle, FALSE, &PreviousState, sizeof(TOKEN_PRIVILEGES), NULL, NULL);// return proc privileges to system
    // DestroyEnvironmentBlock(lpEnvironment);

    // clear memory
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(phToken);
    CloseHandle(phNewToken);
    CloseHandle(TokenHandle);

    //return pId;
    return pId;
}

HANDLE ServiceUtils::GetCurrentUserToken()
{
	PWTS_SESSION_INFO pSessionInfo = 0;
	DWORD dwCount = 0;
	::WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount);
	int session_id = 0;
	for (DWORD i = 0; i < dwCount; ++i)
	{
		WTS_SESSION_INFO si = pSessionInfo[i];
		if (WTSActive == si.State)
		{
			session_id = si.SessionId;
			break;
		}
	}

	::WTSFreeMemory(pSessionInfo);

	HANDLE current_token = 0;
	BOOL bRet = ::WTSQueryUserToken(session_id, &current_token);
	if (bRet == FALSE)
	{
		std::cout << "WTSQueryUserToken failed" << std::endl;
		return 0;
	}

	// 创建一个新的访问令牌来复制一个已经存在的标记
	HANDLE primaryToken = 0;
	bRet = ::DuplicateTokenEx(current_token, TOKEN_ASSIGN_PRIMARY | TOKEN_ALL_ACCESS, 0, SecurityImpersonation, TokenPrimary, &primaryToken);
	if (bRet == FALSE)
	{
		std::cout<< "DuplicateTokenEx failed"<<std::endl;
		return 0;
	}

	::CloseHandle(current_token);

	return primaryToken;
}

BOOL ServiceUtils::launchGUIApplication(std::string app, std::vector<std::string> params)
{
	BOOL bResult = FALSE;
	// 由受限的primaryToken和GetTokenInformation得到不受限的hTheToken
	HANDLE primaryToken = GetCurrentUserToken();
	if (primaryToken == 0)
	{
		std::cout<< "GetCurrentUserToken failed"<<std::endl;
		return FALSE;
	}

	HANDLE hTheToken = NULL;
	DWORD dwSize = 0;
	if (GetTokenInformation(primaryToken, TokenLinkedToken, (VOID*)&hTheToken, sizeof(HANDLE), &dwSize))
	{
		// 让当前线程模拟登陆用户进行操作
		if (ImpersonateLoggedOnUser(hTheToken) == TRUE)
		{
			DWORD dwCreationFlags = HIGH_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
			STARTUPINFO si = { sizeof(si) };
			PROCESS_INFORMATION pi;
			SECURITY_ATTRIBUTES Security1 = { sizeof(Security1) };
			SECURITY_ATTRIBUTES Security2 = { sizeof(Security2) };

			LPVOID pEnv = NULL;
			if (CreateEnvironmentBlock(&pEnv, hTheToken, TRUE))
			{
				dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
			}

			char commandLine[MAX_PATH]={0};
			_tcscpy_s(commandLine, MAX_PATH, " ");
			for (auto item : params) {
				strcat_s(commandLine, MAX_PATH, item.c_str());
			}

			// Launch the process in the client's logon session.
			bResult = CreateProcessAsUser(
				hTheToken,
				(LPSTR)(app.c_str()),
				(LPSTR)(commandLine),
				&Security1,
				&Security2,
				FALSE,
				dwCreationFlags,
				pEnv,
				NULL,
				&si,
				&pi
			);

			if (!bResult)
			{
				std::cout << "CreateProcessAsUser failed" <<std::endl;
			}

			RevertToSelf();

			if (pEnv)
			{
				DestroyEnvironmentBlock(pEnv);
			}
		}
		else
		{
			std::cout << "ImpersonateLoggedOnUser failed" <<std::endl;
		}
		CloseHandle(hTheToken);
	}
	CloseHandle(primaryToken);

	return bResult;
}
