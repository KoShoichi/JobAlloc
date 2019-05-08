// Thread.cpp: implementation of the CThread class.
//
//////////////////////////////////////////////////////////////////////
#include "frame_public.h"
#include "Thread.h"


void *_ThreadEntry(void *pParam)
{
	CThread *pThread = (CThread *)pParam;
    int ret = 0;
    
    pThread->_SetRunStatus(true);
	if(pThread->_Initialize() == 0){  
        pThread->_Run();
	}
	pThread->_Finish();
    pThread->_SetRunStatus(false);
	return NULL;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CThread::CThread()
{
    m_running = false;
    m_context = NULL;
    m_pInitHandle = NULL;
    m_pRunHandle = NULL;
    m_pFinishHandle = NULL;
    m_pStopHandle = NULL;
}

CThread::~CThread()
{
}

int CThread::Start()
{
	pthread_attr_t attr;
	if (pthread_attr_init(&attr) != 0)
		return -1;

	int ret = pthread_create(&m_hThread, &attr, &_ThreadEntry, this);
	if (pthread_attr_destroy(&attr) != 0)
		return -1;
	if (ret != 0)
		return -1;	
	return 0;
}

int CThread::Safe_Stop()
{
    if (m_pStopHandle == NULL)
        return -1;
    return m_pStopHandle(this);
}

int CThread::Join()
{
	return pthread_join(m_hThread, NULL);
}

pthread_t CThread::GetHandle()
{
	return m_hThread;
}

unsigned long CThread::GetSelfThreadId()
{
#ifdef WIN32
	return (unsigned long)pthread_getw32threadid_np(pthread_self());
#else
	return (unsigned long)pthread_self();
#endif
}


bool CThread::IsRunning()
{
	return m_running;
}

int CThread::_Initialize()
{
    if (m_pInitHandle != NULL)
        return m_pInitHandle(this);
    return 0;
}

int CThread::_Run()
{
    if (m_pRunHandle != NULL)
        return m_pRunHandle(this);
    return 0;
}

int CThread::_Finish()
{
    if (m_pFinishHandle != NULL)
        return m_pFinishHandle(this);
    return 0;
}

void CThread::RegisterContext(void *pThreadContext)
{
    m_context = pThreadContext;
}

void * CThread::GetContext()
{
    return m_context;
}


void CThread::RegisterInitHanlde(thread_func_t pHandle)
{
    m_pInitHandle = pHandle;
}

void CThread::RegisterRunHanlde(thread_func_t pHandle)
{
    m_pRunHandle = pHandle;
}

void CThread::RegisterFinishHanlde(thread_func_t pHandle)
{
    m_pFinishHandle = pHandle;
}

void CThread::RegisterStopHanlde(thread_func_t pHandle)
{
    m_pStopHandle = pHandle;
}

void CThread::_SetRunStatus(bool s)
{
    m_running = s;
}


