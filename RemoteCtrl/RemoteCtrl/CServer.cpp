#include"pch.h"
#include"CServer.h"
#include "Tool.h"
#pragma warning(disable:4407)
template<EOperator op>
AcceptOverlapped<op>::AcceptOverlapped(){
    m_worker = ThreadWorker(this, (FUNCTYPE) & AcceptOverlapped<op>::AcceptWorker);
    m_operator = EAccept;
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024);
    m_server = NULL;
}

template<EOperator op>
int AcceptOverlapped<op>::AcceptWorker() {
    INT lLength = 0, rLength = 0;
    if (m_client->GetBufferSize() > 0) {
        sockaddr_in* plocal = NULL, * premote = NULL;
        GetAcceptExSockaddrs(*m_client, 0,
            sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
            (sockaddr**)&plocal, &lLength,//本地地址
            (sockaddr**)&premote, &rLength//远程地址
        );
        memcpy(m_client->GetLocalAddr(), plocal, sizeof(sockaddr_in));
        memcpy(m_client->GetRemoteAddr(), premote, sizeof(sockaddr_in));
        m_server->BindNewSocket(*m_client);
        int ret = WSARecv((SOCKET)*m_client, m_client->RecvWSABuffer(), 1, *m_client, &m_client->flags(), m_client->RecvOverlapped(), NULL);
        if (ret == SOCKET_ERROR && (WSAGetLastError() != WSA_IO_PENDING)) {
            //TODO:报错
            TRACE("ret = %d error = %d\r\n", ret, WSAGetLastError());
        }
        if (!m_server->NewAccept())
        {
            return -2;
        }
    }
    return -1;
}
template <EOperator op>
inline SendOverlapped<op>::SendOverlapped() {
    m_operator = op;
    m_worker = ThreadWorker(this, (FUNCTYPE)&SendOverlapped<op>::SendWorker);
        memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024 * 256);
}

template <EOperator op>
inline RecvOverlapped<op>::RecvOverlapped() {
    m_operator = op;
    m_worker = ThreadWorker(this, (FUNCTYPE)&RecvOverlapped<op>::RecvWorker);
    memset(&m_overlapped, 0, sizeof(m_overlapped));
    m_buffer.resize(1024 * 256);
}

CClient::CClient() 
    :m_isbusy(false), m_flags(0),
    m_overlapped(new ACCEPTOVERLAPPED()),
    m_recv(new RECVOVERLAPPED()),
    m_send(new SENDOVERLAPPED()),
    m_vecSend(this,(SENDCALLBACK)& CClient::SendData)
{
    m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    m_buffer.resize(1024);
    memset(&m_laddr, 0, sizeof(m_laddr));
    memset(&m_raddr, 0, sizeof(m_raddr));
}
void CClient::SetOverlapped(PCLIENT& ptr) {
    m_overlapped->m_client = ptr.get();
    m_recv->m_client = ptr.get();
    m_send->m_client = ptr.get();
}
CClient::operator LPOVERLAPPED() {
    return &m_overlapped->m_overlapped;
}

LPWSABUF CClient::RecvWSABuffer()
{
    return &m_recv->m_wsabuffer;
}

LPWSAOVERLAPPED CClient::RecvOverlapped()
{
    return &m_recv->m_overlapped;
}

LPWSABUF CClient::SendWSABuffer()
{
    return &m_send->m_wsabuffer;
}

LPWSAOVERLAPPED CClient::SendOverlapped()
{
    return &m_send->m_overlapped;
}

int CClient::Recv()
{
    int ret = recv(m_sock, m_buffer.data(), m_buffer.size() - m_used, 0);
    if (ret <= 0)return -1;
    m_used += (size_t)ret;
    CTool::Dump((BYTE*)m_buffer.data(), ret);
    return 0;
}

int CClient::Send(void* buffer, size_t nSize)
{
    std::vector<char> data(nSize);
    memcpy(data.data(), buffer, nSize);
    if (m_vecSend.PushBack(data)) {
        return 0;
    }
    return -1;
}

int CClient::SendData(std::vector<char>& data)
{
    if (m_vecSend.Size() > 0) {
        int ret = WSASend(m_sock, SendWSABuffer(), 1, &m_recvived, m_flags, &m_send->m_overlapped, NULL);
        if (ret != 0 && (WSAGetLastError() != WSA_IO_PENDING)) {
            CTool::ShowError();
            return -1;
        }
    }
    return 0;
}

CServer::~CServer() {
    closesocket(m_sock);
    std::map<SOCKET, PCLIENT>::iterator it = m_client.begin();
    for (; it != m_client.end(); it++) {
        it->second.reset();
    }
    m_client.clear();
    CloseHandle(m_hIOCP);
    m_pool.Stop();
    WSACleanup();
}

bool CServer::StartService()
{
    CreateSocket();
    if (bind(m_sock, (sockaddr*)&m_addr, sizeof(m_addr)) == -1) {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        return false;
    }
    if (listen(m_sock, 3) == -1) {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        return false;
    }
    m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
    if (m_hIOCP == NULL) {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        m_hIOCP = INVALID_HANDLE_VALUE;
        return false;
    }
    CreateIoCompletionPort((HANDLE)m_sock, m_hIOCP, (ULONG_PTR)this, 0);
    m_pool.Invoke();
    m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&CServer::threadIocp));
    if (!NewAccept())return false;
    //m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&CServer::threadIocp));
    //m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&CServer::threadIocp));
    return true;
}

void CServer::BindNewSocket(SOCKET s)
{
    CreateIoCompletionPort((HANDLE)s, m_hIOCP, (ULONG_PTR)this, 0);
}

void CServer::CreateSocket() {
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 2), &WSAData);
    m_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    int opt = 1;
    setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
}

int CServer::threadIocp()
{
    DWORD tranferred = 0;
    ULONG_PTR CompletionKey = 0;
    OVERLAPPED* lpOverlapped = NULL;
    if (GetQueuedCompletionStatus(m_hIOCP, &tranferred, &CompletionKey, &lpOverlapped, INFINITE)) {
        if (CompletionKey != 0) {
            CEOverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, CEOverlapped, m_overlapped);
            TRACE("pOverlapped->m_operator %d \r\n", pOverlapped->m_operator);
            pOverlapped->m_server = this;
            switch (pOverlapped->m_operator) {
            case EAccept:
            {
                ACCEPTOVERLAPPED* pOver = (ACCEPTOVERLAPPED*)pOverlapped;
                m_pool.DispatchWorker(pOver->m_worker);
            }
            break;
            case ERecv:
            {
                RECVOVERLAPPED* pOver = (RECVOVERLAPPED*)pOverlapped;
                m_pool.DispatchWorker(pOver->m_worker);
            }
            break;
            case ESend:
            {
                SENDOVERLAPPED* pOver = (SENDOVERLAPPED*)pOverlapped;
                m_pool.DispatchWorker(pOver->m_worker);
            }
            break;
            case EError:
            {
                ERROROVERLAPPED* pOver = (ERROROVERLAPPED*)pOverlapped;
                m_pool.DispatchWorker(pOver->m_worker);
            }
            break;
            }
        }
        else {
            return -1;
        }
    }
    return 0;
}

bool CServer::NewAccept() {
    PCLIENT pClient(new CClient());
    pClient->SetOverlapped(pClient);
    m_client.insert(std::pair<SOCKET, PCLIENT>(*pClient, pClient));
    if (!AcceptEx(m_sock, *pClient, *pClient, 0, sizeof(sockaddr_in) + 16,
        sizeof(sockaddr_in) + 16, *pClient, *pClient))
    {
        TRACE("%d\r\n", WSAGetLastError());
        if (WSAGetLastError() != WSA_IO_PENDING) {
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            m_hIOCP = INVALID_HANDLE_VALUE;
            return false;
        }
    }
    return true;
}