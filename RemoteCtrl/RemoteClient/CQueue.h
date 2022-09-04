#pragma once

template<class T>
class CQueue
{//线程安全的队列（利用IOCP实现）
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
		int nOperator;//操作
		T strData;//数据
		HANDLE hEvent;//pop操作需要的
		IocpParam(int op, const char* sData) {
			nOperator = op;
			strData = sData;
		}
		IocpParam() {
			nOperator = -1;
		}
	}PPARAM;//Post Parameter 用于投递信息的结构体
	enum {
		QPush,
		QPop,
		QSize.
		QClear
	};
};

