#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//��̬��Ա���������н�����������û�ж��壬����Ҫ��������涨�壬ʵ�����Ǹ���̬��Ա���������ڴ�
CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance();

std::string GetErrorInfo(int wsaErrCode) {
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);//�Ѵ�������и�ʽ���ĺ���
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}