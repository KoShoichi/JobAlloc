#ifndef _FRAME_THREAD_H
#define _FRAME_THREAD_H

#include "pthread.h"

class  CThread;
typedef int (*thread_func_t)(CThread *thread_handle);

/**定义了一个线程对象
*/
class DLL_API CThread
{
public:	
	CThread();
	virtual ~CThread();
	
    //Run方法之前运行，进行初始化工作，返回0（成功）后Run方法才会运行
    void RegisterInitHanlde(thread_func_t pHandle);
    //线程主运行方法
    void RegisterRunHanlde(thread_func_t pHandle);
    //Run方法结束后运行，进行收尾工作
    void RegisterFinishHanlde(thread_func_t pHandle);
    //用于使线程循环体安全退出的方法
    void RegisterStopHanlde(thread_func_t pHandle);

	//启动线程
	int Start();
	//使注册的停止方法来暂停线程的运行
	int Safe_Stop();
	//是否在运行
	bool IsRunning();
	
	//等待线程终止
	int Join();	
	//获得本线程对象存储的线程句柄
	pthread_t GetHandle();
	//获得当前线程ID
	static unsigned long GetSelfThreadId();
    
    //注册托管资源
    void RegisterContext(void *pThreadContext);
    //获取托管的资源内容
    void *GetContext();

    //内部调用
    int _Initialize();
    int _Run();
    int _Finish();
    void _SetRunStatus(bool s);
    
		
private:
	pthread_t m_hThread;
	bool m_running;
    void *m_context;
    thread_func_t m_pInitHandle;
    thread_func_t m_pRunHandle;
    thread_func_t m_pFinishHandle;
    thread_func_t m_pStopHandle;
};



#endif
