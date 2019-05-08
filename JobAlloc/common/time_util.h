#ifndef TIME_UTIL_H
#define TIME_UTIL_H

class DLL_API TimeUtil{
	public:
		//yyyymmdd hh:mm:ss.mi
		static void Now(int *_date, int *_hh=NULL, int *_mm=NULL, int *_ss=NULL, int *_mi=NULL);
		//sleep毫秒
		static void Sleep(unsigned int milliseconds);
		//精确到毫秒，ss返回到当下总的秒数，mi为零碎的毫秒数，total_mi=ss*1000+mi即为总的毫秒数
		static void GetTimeOfDay(long *_ss, long *_mi=NULL, long long *_total_mi=NULL);
		//解析时间字符串转换成整数，09:08:00->90800
		static int ParseTimeStr(const char *pStr);
		//yyyymmdd格式的日期，计算往后n天或者之前n天的日期
		static int NextDate(int _date, int _n);
		//yyyymmdd格式的日期，返回周几，0周日...6周六
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
