#pragma once
#ifndef _FRAME_MUTEX_H
#define _FRAME_MUTEX_H

class DLL_API CMUTEX  
{
public:
	int TryLock();
	void Lock();
	void UnLock();
	CMUTEX();
	virtual ~CMUTEX();
protected:
#ifdef WIN32
   HANDLE m_lock;
#else
   pthread_mutex_t m_lock;
#endif
};


class DLL_API CMutexEvent : public CMUTEX  
{
public:
	void WaitEventTime(unsigned int wait_ms);
	void SetEvent();
	void WaitEvent();
	CMutexEvent();
	virtual ~CMutexEvent();

private:
#ifdef WIN32
   HANDLE m_event;
#else
   pthread_cond_t m_event;
#endif
};



#endif
