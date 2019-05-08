#ifndef _FRAME_THREAD_H
#define _FRAME_THREAD_H

#include "pthread.h"

class  CThread;
typedef int (*thread_func_t)(CThread *thread_handle);

/**������һ���̶߳���
*/
class DLL_API CThread
{
public:	
	CThread();
	virtual ~CThread();
	
    //Run����֮ǰ���У����г�ʼ������������0���ɹ�����Run�����Ż�����
    void RegisterInitHanlde(thread_func_t pHandle);
    //�߳������з���
    void RegisterRunHanlde(thread_func_t pHandle);
    //Run�������������У�������β����
    void RegisterFinishHanlde(thread_func_t pHandle);
    //����ʹ�߳�ѭ���尲ȫ�˳��ķ���
    void RegisterStopHanlde(thread_func_t pHandle);

	//�����߳�
	int Start();
	//ʹע���ֹͣ��������ͣ�̵߳�����
	int Safe_Stop();
	//�Ƿ�������
	bool IsRunning();
	
	//�ȴ��߳���ֹ
	int Join();	
	//��ñ��̶߳���洢���߳̾��
	pthread_t GetHandle();
	//��õ�ǰ�߳�ID
	static unsigned long GetSelfThreadId();
    
    //ע���й���Դ
    void RegisterContext(void *pThreadContext);
    //��ȡ�йܵ���Դ����
    void *GetContext();

    //�ڲ�����
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
