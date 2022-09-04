#pragma once

template<class T>
class CQueue
{//�̰߳�ȫ�Ķ��У�����IOCPʵ�֣�
public:
	CQueue();
	~CQueue();
	bool PushBack(const T& data);
	bool PopFront(T& data);
	size_t Size();
	void Clear();
private:
	static void threadEntry(void* arg);
	void threadMain();
private:
	std::list<T> m_lstData;
	HANDLE m_hCompeletinPort;
	HANDLE m_hThread;
public:
	typedef struct IocpParam {
		int nOperator;//����
		T strData;//����
		HANDLE hEvent;//pop������Ҫ��
		IocpParam(int op, const char* sData) {
			nOperator = op;
			strData = sData;
		}
		IocpParam() {
			nOperator = -1;
		}
	}PPARAM;//Post Parameter ����Ͷ����Ϣ�Ľṹ��
	enum {
		QPush,
		QPop,
		QSize.
		QClear
	};
};

