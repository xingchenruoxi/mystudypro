#pragma once
#include"ClientSocket.h"
#include"RemoteClientDlg.h"
#include"StatusDlg.h"
#include"WatchDialog.h"
#include<map>
#include"resource.h"
#include"Tool.h"

//#define WM_SEND_DATA (WM_USER+2) //��������
#define WM_SHOW_STATUS (WM_USER+3) //չʾ״̬
#define WM_SHOW_WATCH (WM_USER+4) //Զ�̼��
#define WM_SEND_MESSAGE (WM_USER+0x1000) //�Զ�����Ϣ����

class CClientController
{
public:
	//��ȡȫ��Ψһ����
	static CClientController* getInstance();
	//��ʼ������
	int InitController();
	//����
	int Invoke(CWnd *& pMainWnd);
	//������Ϣ
	LRESULT SendMessage(MSG msg);
	//��������������ĵ�ַ
	void UpdateAddress(int nIP, int nPort) {
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}
	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket() {
		CClientSocket::getInstance()->CloseSocket();
	}
	// 1 �鿴���̷���
	// 2 �鿴ָ��Ŀ¼�µ��ļ�
	// 3 ���ļ�
	// 4 �����ļ�
	// 9 ɾ���ļ�
	// 5 ������
	// 6 ������Ļ����==>������Ļ�Ľ�ͼ
	// 7 ����
	// 8 ����
	// 1981 ��������
	//����ֵ����״̬,true�ǳɹ� false��ʧ��
	bool SendCommandPacket(
		HWND hWnd,//���ݰ��յ�����ҪӦ��Ĵ���
		int nCmd, 
		bool bAutoClose = true, 
		BYTE* pData = NULL, 
		size_t nLength = 0);
	int GetImage(CImage& image) {
		CClientSocket* pClient = CClientSocket::getInstance();
		return CTool::Bytes2Image(image, pClient->GetPacket().strData);
	}
	int DownFile(CString strPath);
	void StartWatchScreen();
protected:
	void threadWatchScreen();
	static void threadWatchScreen(void* arg);
	void threadDownloadFile();
	static void threadDownloadEntry(void* arg);
	CClientController():
		m_statusDlg(&m_remoteDlg),
		m_watchDlg(&m_remoteDlg) 
	{
		m_isClosed = true;
		m_hThreadWatch = INVALID_HANDLE_VALUE;
		m_hThreadDownload = INVALID_HANDLE_VALUE;
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	}
	~CClientController() {
		WaitForSingleObject(m_hThread, 100);
	}
	void threadFunc();
	static unsigned __stdcall threadEntry(void* arg);
	static void releaseInstance() {
		if (m_instance != NULL) {
			delete m_instance;
			m_instance = NULL;
		}
	}

	// LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	typedef struct MsgInfo{
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& m) {
			if (this != &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}
	}MSGINFO;
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC> m_mapFunc;

	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;
	HANDLE m_hThreadDownload;
	HANDLE m_hThreadWatch;
	bool m_isClosed;//�����Ƿ�ر�
	CString m_strRemote;//�����ļ���Զ��·��
	CString m_strLocal;//�����ļ��ı��ر���·��
	unsigned m_nThreadID;
	static CClientController* m_instance;
	class CHelper {
	public:
		CHelper() {
			//CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
		}
	};
	static CHelper m_helper;
};

