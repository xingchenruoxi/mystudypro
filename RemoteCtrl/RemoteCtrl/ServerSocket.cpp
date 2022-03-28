#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server;

CServerSocket* CServerSocket::m_instance = NULL;//静态成员变量在类中仅仅是声明，没有定义，所以要在类的外面定义，实际上是给静态成员变量分配内存
CServerSocket::CHelper CServerSocket::m_helper;

CServerSocket* pserver = CServerSocket::getInstance();