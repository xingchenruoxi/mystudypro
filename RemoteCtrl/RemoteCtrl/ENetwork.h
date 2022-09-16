#pragma once
#include"ESocket.h"
#include"CThread.h"
/*
* 1 ���Ĺ��ܵ�����ʲô
* 2 ҵ���߼���ʲô
*/
class ENetwork
{
};

typedef int (*AcceptFunc)(void* arg, const ESOCKET& client);
typedef int (*RecvFunc)(void* arg, const EBuffer& buffer);
typedef int (*SendFunc)(void* arg, ESOCKET& client, int ret);
typedef int (*RecvFromFunc)(void* arg, const EBuffer& buffer,ESockaddrIn& addr);
typedef int (*SendToFunc)(void* arg, const ESockaddrIn& addr, int ret);

class EServerParameter
{
public:
	EServerParameter(
		const std::string& ip = "0.0.0.0",
		short port = 9527,ETYPE type = ETYPE::ETypeTCP,
		AcceptFunc acceptf = NULL,
		RecvFunc recvf = NULL,
		SendFunc sendf = NULL,
		RecvFromFunc recvfromf = NULL,
		SendToFunc sendtof = NULL);
	//����
	EServerParameter& operator<<(AcceptFunc func);
	EServerParameter& operator<<(RecvFunc func);
	EServerParameter& operator<<(SendFunc func);
	EServerParameter& operator<<(RecvFromFunc func);
	EServerParameter& operator<<(SendToFunc func);
	EServerParameter& operator<<(const std::string& ip);
	EServerParameter& operator<<(short port);
	EServerParameter& operator<<(ETYPE type);
	//���
	EServerParameter& operator>>(AcceptFunc& func);
	EServerParameter& operator>>(RecvFunc& func);
	EServerParameter& operator>>(SendFunc& func);
	EServerParameter& operator>>(RecvFromFunc& func);
	EServerParameter& operator>>(SendToFunc& func);
	EServerParameter& operator>>(std::string& ip);
	EServerParameter& operator>>(short& port);
	EServerParameter& operator>>(ETYPE& type);
	//���ƹ��캯�������ں����أ�����ͬ���͵ĸ�ֵ
	EServerParameter(const EServerParameter& param);
	EServerParameter& operator=(const EServerParameter& param);
	std::string m_ip;
	short m_port;
	ETYPE m_type;
	AcceptFunc m_accept;
	RecvFunc m_recv;
	SendFunc m_send;
	RecvFromFunc m_recvfrom;
	SendToFunc m_sendto;
};

class EServer:public ThreadFuncBase
{
public:
	EServer(const EServerParameter& param);//��ʱ���ùؼ�����������Ҫ���ݸ��˿��������ʵ������ȥ����
	~EServer();
	int Invoke(void* args);
	int Send(ESOCKET& client, const EBuffer& buffer);
	int Sendto(ESockaddrIn& addr, const EBuffer& buffer);
	int Stop();
private:
	int threadFunc();
	int threadUDPFunc();
	int threadTCPFunc();
private:
	EServerParameter m_params;
	void* m_args;
	CThread m_thread;
	ESOCKET m_sock;
	std::atomic<bool> m_stop;
};

