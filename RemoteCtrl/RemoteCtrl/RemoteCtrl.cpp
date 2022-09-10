// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include"ServerSocket.h"
#include "Command.h"
#include<conio.h>
#include"CQueue.h"
#include<MSWSock.h>
#include"CServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//#pragma comment( linker, "/subsystem:windows/entry:WinMainCRTStartup" )
//#pragma comment(linker,"/subsystem:windows/entry:mainCRTStartup")
//#pragma comment(linker,"/subsystem:console/entry:mainCRTStartup")
//#pragma comment(linker,"/subsystem:console/entry:WinMainCRTStartup")
// 唯一的应用程序对象
//#define INVOKE_PATH _T("c:\\Windows\\SysWOW64\\RemoteCtrl.exe")
#define INVOKE_PATH _T("C\\Users\\xingchen\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup")

CWinApp theApp;
using namespace std;

//业务和通用
bool ChooseAutoInvoke(const CString& strPath) {
	TCHAR wcsSystem[MAX_PATH] = _T("");
	if (PathFileExists(strPath)) {
		return true;
	}
	CString strInfo = _T("该程序只允许合法的用途！");
	strInfo += _T("继续运行该程序，将使得这台机器处于被监控状态！\n");
	strInfo += _T("如果你不希望这样，请按”取消“按钮，退出程序。\n");
	strInfo += _T("按下”是“按钮，该程序将被复制到你的机器上，并随着系统启动而自动运行！\n");
	strInfo += _T("按下”否“按钮，程序只运行一次，不会在系统内留下任何东西！\n");
	int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
	if (ret == IDYES) {
		//WriteRefisterTable(strPath);
		if (!CTool::WriteStarupDir(strPath)) {
			MessageBox(NULL, _T("复制文件失败，是否权限不足？\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
	}
	else if (ret == IDCANCEL) {
		return false;
	}
	return true;
}
void iocp();
int main()
{
	if (!CTool::Init())return 1;
	iocp();


	//if (CTool::IsAdmin()) {
	//	if (!CTool::Init())return 1;
	//	//MessageBox(NULL, _T("管理员"), _T("用户状态"), 0);
	//	if (ChooseAutoInvoke(INVOKE_PATH)) {
	//		CCommand cmd;
	//		//CServerSocket* pserver = CServerSocket::getInstance();
	//		int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);
	//		switch (ret) {
	//		case -1:
	//			MessageBox(NULL, _T("网络初始化异常，未能成功初始化，请检查网络状态!"), _T("网络初始化失败!"), MB_OK | MB_ICONERROR);
	//			break;
	//		case -2:
	//			MessageBox(NULL, _T("多次无法正常接入用户，自动重试!"), _T("接入用户失败!"), MB_OK | MB_ICONERROR);
	//			break;
	//		}
	//	}
	//}
	//else {
	//	if (CTool::RunAsAdmin() == false) {
	//		CTool::ShowError();
	//		return 1;
	//	}
	//	//MessageBox(NULL, _T("普通用户"), _T("用户状态"), 0);
	//}
	return 0;
}

class COverlapped {
public:
	OVERLAPPED m_overlapped;
	DWORD m_operator;
	char m_buffer[4096];
	COverlapped() {
		m_operator = 0;
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		memset(m_buffer, 0, sizeof(m_buffer));
	}
};

void iocp() {

	CServer server;
	server.StartService();
	getchar();


	//SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);//TCP
	//SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	//if (sock == INVALID_SOCKET) {
	//	CTool::ShowError();
	//	return;
	//}
	//HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, sock, 4);
	//SOCKET client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	//CreateIoCompletionPort((HANDLE)sock, hIOCP, 0, 0);
	//sockaddr_in addr;
	//addr.sin_family = PF_INET;
	//addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	//addr.sin_port = htons(9527);
	//bind(sock, (sockaddr*)&addr, sizeof(addr));
	//listen(sock, 5);
	//COverlapped overlapped;
	//overlapped.m_operator = 1;//accept
	//memset(&overlapped, 0, sizeof(OVERLAPPED));
	//DWORD received = 0;
	//if (AcceptEx(sock, client, overlapped.m_buffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &received, &overlapped.m_overlapped) == FALSE)
	//{
	//	CTool::ShowError();
	//}
	//overlapped.m_operator = 2;
	//WSASend();
	//overlapped.m_operator = 3;
	//WSARecv();
	////开启线程
	//while (true) {//代表一个线程
	//	LPOVERLAPPED pOverlapped = NULL;
	//	DWORD transferred = 0;
	//	DWORD key = 0;
	//	if (GetQueuedCompletionStatus(hIOCP, 0, 0, &pOverlapped, INFINITY)) {
	//		COverlapped* pO = CONTAINING_RECORD(pOverlapped, COverlapped, m_overlapped);
	//		switch (pO->m_operator) {
	//		case 1:
	//			//处理accept的操作
	//			
	//		}
	//	}
	//}
}