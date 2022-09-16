#include "pch.h"
#include "ENetwork.h"

EServer::EServer(const EServerParameter& param):m_stop(false),m_args(NULL)
{
	m_params = param;
	m_thread.UpdateWorker(ThreadWorker(this, (FUNCTYPE)&EServer::threadFunc));
}

EServer::~EServer()
{
	Stop();
}

int EServer::Invoke(void* args)
{
	m_sock.reset(new ESocket(m_params.m_type));
	if (*m_sock == INVALID_SOCKET) {
		printf("%s(%d):%s ERROR!!!(%d)\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError());
		return -1;
	}
	if (m_params.m_type == ETYPE::ETypeTCP) {
		if (m_sock->listen() == -1) {
			return -2;
		}
	}
	ESockaddrIn client;
	if (-1 == m_sock->bind(m_params.m_ip, m_params.m_port)) {
		printf("%s(%d):%s ERROR(%d)!!!\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError());
		return -3;
	}
	if (m_thread.Start() == false)return -4;
	m_args = args;
	return 0;
}

int EServer::Send(ESOCKET& client, const EBuffer& buffer)
{
	int ret = m_sock->send(buffer);//TODO: ������Ȼ�ɹ�����������
	if (m_params.m_send)m_params.m_send(m_args, client, ret);
	return ret;
}

int EServer::Sendto(ESockaddrIn& addr, const EBuffer& buffer)
{
	int ret = m_sock->sendto(buffer, addr);
	if (m_params.m_sendto)m_params.m_sendto(m_args, addr, ret);
	return ret;
}

int EServer::Stop()
{
	if (m_stop == false) {
		m_sock->close();
		m_stop = true;
		m_thread.Stop();
	}
	return 0;
}

int EServer::threadFunc()
{
	if (m_params.m_type == ETYPE::ETypeTCP) {
		return threadTCPFunc();
	}
	else {
		return threadUDPFunc();
	}
	
}

int EServer::threadUDPFunc()
{
	EBuffer buf(1024 * 256);
	ESockaddrIn client;
	int ret = 0;
	while (!m_stop) {
		ret = m_sock->recvfrom(buf, client);
		if (ret > 0) {
			client.update();
			if (m_params.m_recvfrom != NULL) {//*
				m_params.m_recvfrom(m_args, buf, client);
			}
		}
		else {//*
			printf("%s(%d):%s ERROR(%d)!!!ret = %d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
			break;
		}
	}
	if (m_stop == false)m_stop = true;
	m_sock->close();
	printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
	return 0;
}

int EServer::threadTCPFunc()
{
	return 0;
}

EServerParameter::EServerParameter(
	const std::string& ip, short port,ETYPE type, 
	AcceptFunc acceptf, RecvFunc recvf, SendFunc sendf, 
	RecvFromFunc recvfromf, SendToFunc sendtof
)
{
	m_ip = ip;
	m_port = port;
	m_type = type;
	m_accept = acceptf;
	m_recv = recvf;
	m_send = sendf;
	m_recvfrom = recvfromf;
	m_sendto = sendtof;
}

EServerParameter& EServerParameter::operator<<(AcceptFunc func)
{
	m_accept = func;
	return *this;
}

EServerParameter& EServerParameter::operator<<(RecvFunc func)
{
	m_recv = func;
	return *this;
}

EServerParameter& EServerParameter::operator<<(SendFunc func)
{
	m_send = func;
	return *this;
}

EServerParameter& EServerParameter::operator<<(RecvFromFunc func)
{
	m_recvfrom = func;
	return *this;
}

EServerParameter& EServerParameter::operator<<(SendToFunc func)
{
	m_sendto = func;
	return *this;
}

EServerParameter& EServerParameter::operator<<(const std::string& ip)
{
	m_ip = ip;
	return *this;
}

EServerParameter& EServerParameter::operator<<(short port)
{
	m_port = port;
	return *this;
}

EServerParameter& EServerParameter::operator<<(ETYPE type)
{
	m_type = type;
	return *this;
}

EServerParameter& EServerParameter::operator>>(AcceptFunc& func)
{
	func = m_accept;
	return *this;
}

EServerParameter& EServerParameter::operator>>(RecvFunc& func)
{
	func = m_recv;
	return *this;
}

EServerParameter& EServerParameter::operator>>(SendFunc& func)
{
	func = m_send;
	return *this;
}

EServerParameter& EServerParameter::operator>>(RecvFromFunc& func)
{
	func = m_recvfrom;
	return *this;
}

EServerParameter& EServerParameter::operator>>(SendToFunc& func)
{
	func = m_sendto;
	return *this;
}

EServerParameter& EServerParameter::operator>>(std::string& ip)
{
	ip = m_ip;
	return *this;
}

EServerParameter& EServerParameter::operator>>(short& port)
{
	port = m_port;
	return *this;
}

EServerParameter& EServerParameter::operator>>(ETYPE& type)
{
	type = m_type;
	return *this;
}

EServerParameter::EServerParameter(const EServerParameter& param)
{
	m_ip = param.m_ip;
	m_port = param.m_port;
	m_type = param.m_type;
	m_accept = param.m_accept;
	m_recv = param.m_recv;
	m_send = param.m_send;
	m_recvfrom = param.m_recvfrom;
	m_sendto = param.m_sendto;
}

EServerParameter& EServerParameter::operator=(const EServerParameter& param)
{
	if (this != &param) {
		m_ip = param.m_ip;
		m_port = param.m_port;
		m_type = param.m_type;
		m_accept = param.m_accept;
		m_recv = param.m_recv;
		m_send = param.m_send;
		m_recvfrom = param.m_recvfrom;
		m_sendto = param.m_sendto;
	}
	return *this;
}
