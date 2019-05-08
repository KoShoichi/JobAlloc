#ifndef TIME_UTIL_H
#define TIME_UTIL_H

class DLL_API TimeUtil{
	public:
		//yyyymmdd hh:mm:ss.mi
		static void Now(int *_date, int *_hh=NULL, int *_mm=NULL, int *_ss=NULL, int *_mi=NULL);
		//sleep����
		static void Sleep(unsigned int milliseconds);
		//��ȷ�����룬ss���ص������ܵ�������miΪ����ĺ�������total_mi=ss*1000+mi��Ϊ�ܵĺ�����
		static void GetTimeOfDay(long *_ss, long *_mi=NULL, long long *_total_mi=NULL);
		//����ʱ���ַ���ת����������09:08:00->90800
		static int ParseTimeStr(const char *pStr);
		//yyyymmdd��ʽ�����ڣ���������n�����֮ǰn�������
		static int NextDate(int _date, int _n);
		//yyyymmdd��ʽ�����ڣ������ܼ���0����...6����
		static int GetDayOfWeek(int _date);
};

class CThread;
class CLinkedList;
class CMUTEX;
class CTimer;

typedef struct _time_node_t time_node_t;
typedef int (*ontimer_func_t)(time_node_t *pTimeNode);

typedef struct _time_node_t
{
    ontimer_func_t pFunc;
    int nIDEvent;
    long nElapse;
    long long tExpire;
    void *pData;
    void *pContext;
    int nTotalTimes;
    int nTimes;
} time_node_t;



class DLL_API CTimer
{
public:
    CTimer();
    ~CTimer();
    int RegisterTimer(ontimer_func_t pFunc, int nIDEvent, long nElapse, void *pData, void *pContext, int _times);
    int RemoveTimer(ontimer_func_t pFunc, int nIDEvent);

public:
    CThread *m_pThread;
    CLinkedList *m_pTimeList;
    CMUTEX *m_pMutex;

    bool m_running;    
    volatile long long m_CurMilTime;
};


#endif
