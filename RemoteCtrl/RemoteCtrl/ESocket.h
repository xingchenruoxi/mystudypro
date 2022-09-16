#pragma once
#include<Winsock2.h>
#include<memory>
enum  class ETYPE
{
	ETypeTCP =1,
	ETypeUDP
};

class ESockaddrIn
{
public:
	ESockaddrIn() {
		memset(&m_addr, 0, sizeof(m_addr));
		m_port = -1;
	}
	ESockaddrIn(sockaddr_in addr) {
		memcpy(&m_addr, &addr, sizeof(addr));
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = ntohs(m_addr.sin_port);
	}
	ESockaddrIn(UINT nIP, short nPort) {
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(nPort);
		m_addr.sin_addr.s_addr = htonl(nIP);
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = nPort;

	}
	ESockaddrIn(const std::string& strIP, short nPort) {
		m_ip = strIP;
		m_port = nPort;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(nPort);;
		m_addr.sin_addr.s_addr = inet_addr(strIP.c_str());
	}
	ESockaddrIn(const ESockaddrIn& addr) {
		memcpy(&m_addr, &addr.m_addr, sizeof(m_addr));
		m_ip = addr.m_ip;
		m_port = addr.m_port;
	}
	ESockaddrIn& operator=(const ESockaddrIn& addr) {
		if (this != &addr) {
			memcpy(&m_addr, &addr.m_addr, sizeof(m_addr));
			m_ip = addr.m_ip;
			m_port = addr.m_port;
		}
		return *this;
	}
	operator sockaddr* () const{
		return (sockaddr*)&m_addr;
	}
	operator void* ()const {
		return (void*)&m_addr;
	}
	void update() {
		m_ip = inet_ntoa(m_addr.sin_addr);
		m_port = ntohs(m_addr.sin_port);
	}
	std::string GetIp()const {
		return m_ip;
	}
	short GetPort()const { return m_port; }
	inline int size() const { return sizeof(sockaddr_in); }
private:
	sockaddr_in m_addr;
	std::string m_ip;
	short m_port;
};

class EBuffer :public std::string
{
public:
	EBuffer(const char* str) {
		resize(strlen(str));
		memcpy((void*)c_str(), str, size());
	}
	EBuffer(size_t size = 0) :std::string() {
		if (size > 0) {
			resize(size);
			memset(*this, 0, this->size());
		}
	}
	EBuffer(void* buffer, size_t size) :std::string() {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
	~EBuffer() {
		std::string::~basic_string();
	}
	operator char* () const { return (char*)c_str(); }
	operator const char* () const { return c_str(); }
	operator BYTE* () const { return (BYTE*)c_str(); }
	operator void* () const { return (void*)c_str(); }
	void Update(void* buffer, size_t size) {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
};

class ESocket
{
public:
	ESocket(ETYPE nType = ETYPE::ETypeTCP,int nPortocol = 0) {
		m_socket = socket(PF_INET, (int)nType, nPortocol);
		m_type = nType;
		m_protocol = nPortocol;
	}
	ESocket(const ESocket& sock) {
		m_socket = socket(PF_INET, (int)sock.m_type, m_protocol);
		m_type = sock.m_type;
		m_protocol = sock.m_protocol;
		m_addr = sock.m_addr;
	}
	~ESocket() {
		close();
	}
	ESocket& operator=(const ESocket& sock) {
		if (this != &sock) {
			m_socket = socket(PF_INET, (int)sock.m_type, m_protocol);
			m_type = sock.m_type;
			m_protocol = sock.m_protocol;
			m_addr = sock.m_addr;
		}
		return *this;
	}
	operator SOCKET() const { return m_socket; }
	operator SOCKET() { return m_socket; }
	bool operator==(SOCKET sock) const {
		return m_socket == sock;
	}
	int listen(int backlog = 5) {
		if (m_type != ETYPE::ETypeTCP) return -1;
		return ::listen(m_socket, backlog);
	}
	int bind(const std::string& ip, short port) {
		m_addr = ESockaddrIn(ip, port);
		return ::bind(m_socket, m_addr, m_addr.size());
	}
	int accept() {

	}
	int connect(const std::string& ip, short port) {

	}
	int send(const EBuffer& buffer){
		return ::send(m_socket, buffer, buffer.size(), 0);
	}
	int recv(EBuffer& buffer) {
		return ::recv(m_socket, buffer, buffer.size(), 0);
	}
	int sendto(const EBuffer& buffer,const ESockaddrIn& to) {
		return ::sendto(m_socket, buffer, buffer.size(), 0, to ,to.size());
	}
	int recvfrom(EBuffer& buffer, ESockaddrIn& from) {
		int len = from.size();
		int ret = ::recvfrom(m_socket, buffer, buffer.size(), 0, from, &len);
		if (ret > 0) {
			from.update();
		}
		return ret;
	}
	void close() {
		if (m_socket != INVALID_SOCKET) {
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
	}
private:
	SOCKET m_socket;
	ETYPE m_type;
	int m_protocol;
	ESockaddrIn m_addr;
};

typedef std::shared_ptr<ESocket> ESOCKET;