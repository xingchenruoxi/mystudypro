#pragma once
#include"pch.h"
#include<atomic>
#include<vector>
#include<mutex>
#include<Windows.h>

class ThreadFuncBase{};
typedef int (ThreadFuncBase::* FUNCTYPE)();
class ThreadWorker {
public:
	ThreadWorker():thiz(NULL),func(NULL){}

	ThreadWorker(ThreadFuncBase* obj,FUNCTYPE f):thiz(obj),func(f){}

	ThreadWorker(const ThreadWorker& worker) {
		thiz = worker.thiz;
		func = worker.func;
	}
	ThreadWorker& operator=(const ThreadWorker& worker) {
		if (this != &worker) {
			thiz = worker.thiz;
			func = worker.func;
		}
		return *this;
	}

	int operator()() {
		if (IsValid()) {
			return (thiz->*func)();
		}
		return -1;
	}
	bool IsValid() const{
		return(thiz != NULL) && (func != NULL);
	}
private:
	ThreadFuncBase* thiz;
	FUNCTYPE func;
};
class CThread
{
public:
	CThread(){
		m_hThread = NULL;
		
	}
	~CThread() {
		Stop();
	}
	//true��ʾ�ɹ� false��ʾʧ��
	bool Start() {
		m_bStatus = true;
		m_hThread = (HANDLE)_beginthread(CThread::ThreadEntry, 0, this);
		if (!IsValid()) {
			m_bStatus = false;
		}
		return m_bStatus;
	}
	bool IsValid() {//����true��ʾ��Ч ����false��ʾ�߳��쳣�����Ѿ���ֹ
		if (m_hThread == NULL || (m_hThread == INVALID_HANDLE_VALUE))return false;
		return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
	}
	bool Stop() {
		if (m_bStatus == false)return true;
		m_bStatus = false;
		bool ret =  WaitForSingleObject(m_hThread, INFINITE) == WAIT_OBJECT_0;
		UpdateWorker();
		return ret;
	}

	void UpdateWorker(const ::ThreadWorker& worker = ::ThreadWorker()) {
		if (!worker.IsValid()) {
			m_worker.store(NULL);
			return;
		}
		if (m_worker.load() != NULL) {
			::ThreadWorker* pWorker = m_worker.load();
			m_worker.store(NULL);
			delete pWorker;
		}
		m_worker.store(new ::ThreadWorker(worker));
	}

	//true��ʾ���� false��ʾ�Ѿ������˹���
	bool IsIdle() {
		return !m_worker.load()->IsValid();
	}

private:
	void ThreadWorker() {
		while (m_bStatus) {
			::ThreadWorker worker = *m_worker.load();
			if (worker.IsValid()) {
				int ret = worker();
				if (ret != 0) {
					CString str;
					str.Format(_T("thread found warning code %d\r\n"), ret);
					OutputDebugString(str);
				}
				if (ret < 0) {
					m_worker.store(NULL);
				}
			}
			else
			{
				Sleep(1);
			}
		}
	}
	static void ThreadEntry(void* arg) {
		CThread* thiz = (CThread*)arg;
		if (thiz) {
			thiz->ThreadWorker();
		}
		_endthread();
	}
private:
	HANDLE m_hThread;
	bool m_bStatus;//false ��ʾ�߳̽�Ҫ�رգ� true ��ʾ�߳���������
	std::atomic<::ThreadWorker*> m_worker;
};

class CThreadPool
{
public:
	CThreadPool(size_t size) {
		m_threads.resize(size);
		for (size_t i = 0; i < size; i++) {
			m_threads[i] = new CThread();
		}
	}
	CThreadPool(){}
	~CThreadPool(){
		Stop();
		m_threads.clear();
	}
	bool Invoke() {
		bool ret = true;
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->Start() == false) {
				ret = false;
				break;
			}
		}
		if (ret == false) {
			for (size_t i = 0; i < m_threads.size(); i++) {
				m_threads[i]->Stop();
			}
		}
		return ret;
	}
	void Stop() {
		for (size_t i = 0; i < m_threads.size(); i++) {
			m_threads[i]->Stop();
		}
	}

	//����-1��ʾ����ʧ�ܣ������̶߳���æ ���ڵ���0����ʾ��n���̷߳��������������
	int DispatchWorker(const ThreadWorker& worker) {
		int index = -1;
		m_lock.lock();
		for (size_t i = 0; i < m_threads.size(); i++) {
			if (m_threads[i]->IsIdle()) {
				m_threads[i]->UpdateWorker(worker);
				index = 1;
				break;
			}
		}
		m_lock.unlock();
		return index;
	}

	bool CheckThreadValid(size_t index) {
		if (index < m_threads.size()) {
			return m_threads[index]->IsValid();
		}
		return false;
	}
private:
	std::mutex m_lock;
	std::vector<CThread*> m_threads;
};