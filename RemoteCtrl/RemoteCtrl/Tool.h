#pragma once
class CTool
{
public:
    static void Dump(BYTE* pData, size_t nSize) {
        std::string strOut;
        for (size_t i = 0; i < nSize; i++) {
            char buf[8] = "";
            snprintf(buf, sizeof(buf), "%02x", pData[i] & 0xFF);
            strOut += buf;
        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());
    }
    static bool IsAdmin() {
		HANDLE hToken = NULL;
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		{
			ShowError();
			return false;
		}
		TOKEN_ELEVATION eve;
		DWORD len = 0;
		if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE) {
			ShowError();
			return false;
		}
		CloseHandle(hToken);
		if (len == sizeof(eve)) {
			return eve.TokenIsElevated;
		}
		printf("length of tokeninformation is %d\r\n", len);
		return false;
	}

	static bool RunAsAdmin() {
		//获取管理员权限，使用改权限创建进程
		//本地策略组 开启Administrator账户 禁止空密码只能登录本地控制台
		//HANDLE hToken = NULL;
		////LOGON32_LOGON_BATCH
		//BOOL ret = LogonUser(L"xingchen", NULL, L"15350286636LNX", LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken);
		//if (!ret) {
		//	ShowError();
		//	MessageBox(NULL, _T("登录错误！"), _T("程序错误"), 0);
		//	exit(0);
		//}
		//OutputDebugString(L"Logon adminstrator success!\r\n");
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		TCHAR sPath[MAX_PATH] = _T("");
		/*GetCurrentDirectory(MAX_PATH, sPath);
		CString strCmd = sPath;
		strCmd += _T("RemoteCtrl.exe");*/
		GetModuleFileName(NULL, sPath, MAX_PATH);
		//ret = CreateProcessWithTokenW(hToken, LOGON_WITH_PROFILE, NULL, (LPWSTR)(LPCWSTR)strCmd, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
		BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL,
			NULL, LOGON_WITH_PROFILE, NULL,
			(LPWSTR)(LPCWSTR)sPath, CREATE_UNICODE_ENVIRONMENT,
			NULL, NULL, &si, &pi);
		//CloseHandle(hToken);
		if (!ret) {
			ShowError();//TODO:去掉调试信息
			MessageBox(NULL, sPath, _T("程序错误"), 0);//TODO:去掉调试信息
			return false;
		}
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return true;
	}

	static void ShowError() {
		LPWSTR lpMessageBuf = NULL;
		//strerror(errno);//标准c语言库
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&lpMessageBuf, 0, NULL);
		OutputDebugString(lpMessageBuf);
		MessageBox(NULL, lpMessageBuf, _T("发生错误"), 0);
		LocalFree(lpMessageBuf);
	}
	/*
* 改bug的思路
* 0 观察现象
* 1 先确定范围
* 2 分析错误的可能性
* 3 调试或者打日志，排查错误
* 4 处理错误
* 5 验证/长时间验证/多次验证/多条件的验证
*/
	static BOOL WriteStarupDir(const CString& strPath) 
	{//通过修改开机启动文件夹来实现开机启动
		TCHAR sPath[MAX_PATH] = _T("");
		GetModuleFileName(NULL, sPath, MAX_PATH);
		//fopen CFile system(copy) CopyFile OpenFile
		return CopyFile(sPath, strPath, FALSE);
	}
	//开机启动的时候，程序的权限是跟随启动用户的
	//如果两者权限不一致，则会导致程序启动失败
	//开机启动对环境变量有影响，如果依赖dll（动态库）,则可能启动失败
	// 解决方法：
	//【复制这些到system32下面或者sysWOW64下面】
	//system32下面，多是64位程序，sysWOW64下面，多是32位程序
	//【使用静态库，而非动态库】
	static bool WriteRefisterTable(const CString& strPath) 
	{//通过修改注册表来实现开机启动
		CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
		TCHAR sPath[MAX_PATH] = _T("");
		//std::string strExe = "\\RemoteCtrl.exe ";
		GetModuleFileName(NULL, sPath, MAX_PATH);
		//fopen CFile system(copy) CopyFile OpenFile
		BOOL ret = CopyFile(sPath, strPath, FALSE);
		if (ret == FALSE) {
			MessageBox(NULL, _T("复制文件失败，是否权限不足？\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
		/*std::string strCmd = "mklink " + std::string(sSys) + strExe + std::string(sPath) + strExe;
		int ret = system(strCmd.c_str());
		TRACE("ret = %d\r\n", ret);*/
		HKEY hKey = NULL;
		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
		if (ret != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
		ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
		if (ret != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败！"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
		RegCloseKey(hKey);
		return true;
	}

	static bool Init() 
	{//用于带mfc命令行项目初始化（通用）
		HMODULE hModule = ::GetModuleHandle(nullptr);
		if (hModule == nullptr) {
			wprintf(L"错误: GetModuleHandle 失败\n");
			return false;
		}
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: 在此处为应用程序的行为编写代码。
			wprintf(L"错误: MFC 初始化失败\n");
			return false;
		}
		return true;
	}
};

