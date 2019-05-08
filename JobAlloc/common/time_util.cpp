#include "frame_public.h"
#include "time_util.h"
#include "string_util.h"
#include "Thread.h"
#include "linked_list.h"
#include "mutex.h"

void TimeUtil::Now(int *_date, int *_hh, int *_mm, int *_ss, int *_mi)
{
#ifdef WIN32
	SYSTEMTIME time;
	GetLocalTime(&time);
	if (_date != NULL) *_date = time.wYear*10000 + time.wMonth*100 + time.wDay;
	if (_hh != NULL) *_hh = time.wHour;
	if (_mm != NULL) *_mm = time.wMinute;
	if (_ss != NULL) *_ss = time.wSecond;
	if (_mi != NULL) *_mi = time.wMilliseconds;
#else
	timeval time;
	gettimeofday(&time, NULL);
	long millis = (time.tv_sec * 1000) + (time.tv_usec / 1000);
#endif

}


#define MAXUSLEEPUNIT 500000
void TimeUtil::Sleep(unsigned int milliseconds)
{
#ifdef WIN32
    ::Sleep(milliseconds);
#else
    unsigned int i;
    milliseconds *= 1000;
    do {
        if (milliseconds>MAXUSLEEPUNIT) {
            i = MAXUSLEEPUNIT;
        }
        else {
            i = milliseconds;
        }
        milliseconds -= i;
        usleep(i);
    } while(milliseconds>100);
#endif
}

void TimeUtil::GetTimeOfDay(long *_ss, long *_mi/*=NULL*/, long long *_total_mi/*=NULL*/)
{
#ifdef WIN32
    time_t _clock;
    struct tm _tm;
    SYSTEMTIME _wtm;
    GetLocalTime(&_wtm);
    _tm.tm_year = _wtm.wYear - 1900;
    _tm.tm_mon = _wtm.wMonth - 1;
    _tm.tm_mday = _wtm.wDay;
    _tm.tm_hour = _wtm.wHour;
    _tm.tm_min = _wtm.wMinute;
    _tm.tm_sec = _wtm.wSecond;
    _tm.tm_isdst = -1;
    _clock = mktime(&_tm);

    if (_ss != NULL)
        *_ss = (long)_clock;
    if (_mi != NULL)
        *_mi = _wtm.wMilliseconds;
    if (_total_mi != NULL)
        *_total_mi = (long long)_clock*1000 + _wtm.wMilliseconds;
#endif
}

int TimeUtil::ParseTimeStr(const char *pStr)
{
    int ret = 0;
    char buf[10] = {0};
    const char *_p = pStr;
    do {
        _p = StringUtil::split(_p, ':', buf);
        ret = ret*100 + atoi(buf);
    } while(_p);
    return ret;
}

int TimeUtil::NextDate(int _date, int _n)
{
    int yyyy = _date / 10000;
    int mm = (_date%10000) / 100;
    int dd = _date % 100;
    struct tm _tm;
    _tm.tm_year = yyyy - 1900;
    _tm.tm_mon = mm - 1;
    _tm.tm_mday = dd;
    _tm.tm_hour = 12;       //放到一日的当中，减小奇怪的时间误差
    _tm.tm_min = 0;
    _tm.tm_sec = 0;
    _tm.tm_isdst = -1;
    time_t _clock = mktime(&_tm);

    const int ONE_DAY = 24 * 60 * 60;
    time_t date_seconds = _clock + (_n * ONE_DAY);
    struct tm *pNewTM = localtime(&date_seconds);
    return ((pNewTM->tm_year+1900)*10000 + (pNewTM->tm_mon+1)*100 + pNewTM->tm_mday);
}

int TimeUtil::GetDayOfWeek(int _date)
{
    int yyyy = _date / 10000;
    int mm = (_date%10000) / 100;
    int dd = _date % 100;
    struct tm _tm;
    _tm.tm_year = yyyy - 1900;
    _tm.tm_mon = mm - 1;
    _tm.tm_mday = dd;
    _tm.tm_hour = 12;       //放到一日的当中，减小奇怪的时间误差
    _tm.tm_min = 0;
    _tm.tm_sec = 0;
    _tm.tm_isdst = -1;
    time_t _clock = mktime(&_tm);
    mktime(&_tm);
    return _tm.tm_wday;  //  0...6 for Sunday...Saturday
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void AddTimerNode(CTimer *pTimer, time_node_t *pNode)
{
    CLinkedList *pTimeList = pTimer->m_pTimeList;
    NODEHANDLE it = pTimeList->HeadNode();
    while(it != NULL) {
        time_node_t *pValue = (time_node_t *)pTimeList->GetValue(it);
        if (pValue->tExpire > pNode->tExpire) {
            pTimeList->InsertBefore(it, (void *)pNode, default_free_node_func);
            return;
        }
        it = pTimeList->NextNode(it);
    }
    pTimeList->InsertAfter(it, (void *)pNode, default_free_node_func);
}

static int TimerRun(CThread *pThread)
{
    CTimer *pTimer = (CTimer *)pThread->GetContext();
    CLinkedList *pTimeList = pTimer->m_pTimeList;
    long long tmp_CurMilTime = 0;
    bool bTrigger = false;
    while(pTimer->m_running)
    {      
        TimeUtil::GetTimeOfDay(NULL, NULL, &tmp_CurMilTime);
        pTimer->m_CurMilTime = tmp_CurMilTime;
        
        pTimer->m_pMutex->Lock();
        NODEHANDLE it = pTimeList->HeadNode();
        while(it != NULL) {
            time_node_t *pValue = (time_node_t *)pTimeList->GetValue(it);
            if (pValue->tExpire > pTimer->m_CurMilTime)
                break;  //链表是按照到期时间先后排序的，所以此节点未到期则之后的节点都不会到期

            bTrigger = true;
            NODEHANDLE tmp_it = it;
            it = pTimeList->NextNode(it);
            pTimeList->PopNode(tmp_it);

            if (pValue->nTotalTimes > 0 && pValue->nTimes >= pValue->nTotalTimes) {
                free(pValue);
                continue;
            }
            if(pValue->pFunc != NULL){
                pValue->nTimes += 1;
                pValue->pFunc(pValue);
                pValue->tExpire = pTimer->m_CurMilTime + pValue->nElapse;
                if (pValue->nTotalTimes > 0 && pValue->nTimes >= pValue->nTotalTimes) {
                    free(pValue);
                    continue;
                }
                else {
                    AddTimerNode(pTimer, pValue);
                }                
            }
            else {
                free(pValue);
                continue;
            }
        }
        pTimer->m_pMutex->UnLock();
    }

    if (bTrigger)
        bTrigger = false;
    else
        TimeUtil::Sleep(1);
    return 0;
}

CTimer::CTimer()
{    
    m_pThread = new CThread;
    m_pTimeList = new CLinkedList;
    m_pMutex = new CMUTEX;
    m_running = true;
    m_CurMilTime = 0;

    m_pThread->RegisterContext((void *)this);
    m_pThread->RegisterRunHanlde(TimerRun);    
    m_pThread->Start();
}

CTimer::~CTimer()
{
    m_running = false;
    m_pThread->Join();

    delete m_pMutex;
    delete m_pTimeList;
    delete m_pThread;
}

int CTimer::RegisterTimer(ontimer_func_t pFunc, int nIDEvent, long nElapse, void *pData, void *pContext, int _times)
{
    time_node_t *pNewTimerNode = (time_node_t *)calloc(1, sizeof(time_node_t));
    if (pNewTimerNode == NULL)
        return -1;
    pNewTimerNode->pFunc = pFunc;
    pNewTimerNode->nIDEvent = nIDEvent;
    pNewTimerNode->nElapse = nElapse;
    pNewTimerNode->pData = pData;
    pNewTimerNode->pContext = pContext;
    pNewTimerNode->tExpire = m_CurMilTime + nElapse;
    pNewTimerNode->nTotalTimes = _times;
    pNewTimerNode->nTimes = 0;
    m_pMutex->Lock();
    AddTimerNode(this, pNewTimerNode);
    m_pMutex->UnLock();
    return 0;
}

int CTimer::RemoveTimer(ontimer_func_t pFunc, int nIDEvent)
{
    m_pMutex->Lock();
    NODEHANDLE it = m_pTimeList->HeadNode();
    while(it != NULL) {
        time_node_t *pValue = (time_node_t *)m_pTimeList->GetValue(it);
        if (pValue->pFunc == pFunc && (nIDEvent==0 || pValue->nIDEvent==nIDEvent)) {
            pValue->pFunc = NULL;
        }
        it = m_pTimeList->NextNode(it);
    }
    m_pMutex->UnLock();
    return 0;
}


