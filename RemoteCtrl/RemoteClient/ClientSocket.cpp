#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;//��̬��Ա���������н�����������û�ж��壬����Ҫ��������涨�壬ʵ�����Ǹ���̬��Ա���������ڴ�
CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance();