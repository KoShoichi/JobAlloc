#include "frame_public.h"
#include "applog.h"
#include "Thread.h"
#include "file_util.h"
#include "time_util.h"
#include "sequence.h"

#define BEGIN_LOG_MSG "--------------------Begin Log--------------------\n"

class CLogHandle;
static CLogHandle *g_loghandle = NULL;


typedef struct _thread_log_context_t {
    char full_name_[MAX_PATH];
    FILE *fp;
    int cur_date_;      //日志当前使用的日期，日期切换时修改
    bool is_main_thread_;   //用于区分是否是主线程，主线程日志文件不带线程号等区别用
} thread_log_context_t;

class CLogHandle {
public:
    CLogHandle(const char *raw_name, int log_level, unsigned int max_threads=1);
    ~CLogHandle();

    int Open();
    void Close();
    //打印小于等于log_level_的日志
    void Log(int _level, const char *fmt, va_list args);

public:
    char path_name_[MAX_PATH];
    char base_name_[MAX_PATH];
    char ext_name_[MAX_PATH];

    
    int log_level_;
    thread_log_context_t *thread_log_context_list_;
    pthread_key_t thread_key_;
    Sequence log_id;	//日志序号，多线程同一主handle的日志继承同一份序号，用于不同文件确定日志的先后次序
    Sequence thread_idx;
    unsigned int max_threads_;
};

CLogHandle::CLogHandle(const char *raw_name, int log_level, unsigned int max_threads/*=1*/) : log_id(0L), thread_idx(0L)
{
    log_level_ = log_level;
    if (max_threads <= 0)
        max_threads_ = 1;
    else if (max_threads > 1000)
        max_threads_ = 1000;
    else
        max_threads_ = max_threads;
    
    pthread_key_create(&thread_key_, NULL);

    FileUtil::parse_file_name(raw_name, path_name_, base_name_, ext_name_);
    if (strlen(path_name_) == 0)
        strcpy(path_name_, "./log/");
    if (strlen(base_name_) == 0)
        strcpy(base_name_, "applog");
    if (strlen(ext_name_) == 0)
        strcpy(ext_name_, "log");
}

CLogHandle::~CLogHandle()
{
    this->Close();
    pthread_key_delete(thread_key_);
}

int CLogHandle::Open()
{
    thread_log_context_list_ = (thread_log_context_t *)calloc(max_threads_, sizeof(thread_log_context_t));
    thread_idx.Set(0L);
    thread_log_context_t *p = thread_log_context_list_;
    int _date = 0;
    TimeUtil::Now(&_date);

    if (0 != FileUtil::ensure_path(path_name_))
        return -1;

    //主线程的日志不带线程号    
    sprintf(p->full_name_, "%s%s_%d.%s", path_name_, base_name_, _date, ext_name_);
    p->fp = fopen(p->full_name_, "a");
    if (p->fp == NULL) {
        return -1;
    }        
    fprintf(p->fp, BEGIN_LOG_MSG);
    fflush(p->fp);
    p->cur_date_ = _date;
    p->is_main_thread_ = true;
    pthread_setspecific(thread_key_, p);
    return 0;
}

void CLogHandle::Close()
{
    int max_cur = (int)thread_idx.Get();
    for (int i=0; i<=max_cur; ++i) {
        thread_log_context_t *p = thread_log_context_list_ + i;
        if (p->fp != NULL) {
            fflush(p->fp);
            fclose(p->fp);
            p->fp = NULL;
        }
    }
}

void CLogHandle::Log(int _level, const char *fmt, va_list args)
{    
	//单纯屏幕打印
	if ((_level == LOG_STD_T) || (_level == LOG_PT_T))
	{
		vprintf(fmt, args);
		printf("\n");
		fflush(stdout);
		if (_level == LOG_PT_T) return;
	}

    int _date, _hh, _mm, _ss, _mi;
    TimeUtil::Now(&_date, &_hh, &_mm, &_ss, &_mi);

    thread_log_context_t *p = (thread_log_context_t *)pthread_getspecific(thread_key_);
    if (p == NULL) {
        //新线程中第一次写日志，创建日志文件句柄，只可能是子线程进入
        //线程数超过max_threads_，则循环使用日志资源
        int _index = thread_idx.AddAndGet() % max_threads_;
        p = thread_log_context_list_ + _index;
        sprintf(p->full_name_, "%s%s_%d_%ld.%s", path_name_, base_name_, _date, CThread::GetSelfThreadId(), ext_name_);
        p->fp = fopen(p->full_name_, "a");
        if (p->fp == NULL)
            return;
        fprintf(p->fp, BEGIN_LOG_MSG);
        fflush(p->fp);
        p->cur_date_ = _date;
        p->is_main_thread_ = false;
        pthread_setspecific(thread_key_, p);
    }
    if (p->cur_date_ != _date) {
        if (p->fp != NULL)
            fclose(p->fp);
        if (p->is_main_thread_)
            sprintf(p->full_name_, "%s%s_%d.%s", path_name_, base_name_, _date, ext_name_);
        else
            sprintf(p->full_name_, "%s%s_%d_%ld.%s", path_name_, base_name_, _date, CThread::GetSelfThreadId(), ext_name_);
        p->fp = fopen(p->full_name_, "a");
        if (p->fp == NULL)
            return;
        p->cur_date_ = _date;
    }

	
	if (_level <= log_level_)
	{
        //[yyyymmdd hh:mm:ss.si][level][logid]: text
        char buf[1024] = {0};	
        sprintf(buf, "[%08d %02d:%02d:%02d.%03d][%d][%lld]: ", _date, _hh, _mm, _ss, _mi, _level, log_id.AddAndGet());
		fprintf(p->fp, "%s", buf);
		vfprintf(p->fp, fmt, args);
		fprintf(p->fp, "\n");
		fflush(p->fp);
	}
    //日期变化则记录到新的日志文件
	/*sprintf(strdate, "%04d%02d%02d", infotime->tm_year+1900, infotime->tm_mon+1, infotime->tm_mday);
	if (strcmp(strdate, filedate) != 0)
	{
	fclose(fp);
	strcpy(filedate, strdate);
	char newfile[512] = {0};
	sprintf(newfile, "%s.%s", filename, filedate);
	fp = fopen(newfile, "a");
	}*/
}


void      CAppLog::CreateGlobalHandle(const char *log_name, int log_level, unsigned int max_threads/*=1*/)
{
    g_loghandle = new CLogHandle(log_name, log_level, max_threads);
}

LOGHANDLE CAppLog::CreateLogHandle(const char *log_name, int log_level, unsigned int max_threads/*=1*/)
{
    CLogHandle *p = new CLogHandle(log_name, log_level, max_threads);
    return (LOGHANDLE)p;
}

int CAppLog::Open(LOGHANDLE _handle /*= NULL*/)
{
    CLogHandle *p = (CLogHandle *)_handle;
    if (p == NULL) {
        if (g_loghandle == NULL)
            return -1;
        p = g_loghandle;
    }
    return p->Open();
}

void CAppLog::Close(LOGHANDLE _handle /*= NULL*/)
{
    CLogHandle *p = (CLogHandle *)_handle;
    if (p == NULL) {
        if (g_loghandle == NULL)
            return;
        g_loghandle->Close();
        delete g_loghandle;
        g_loghandle = NULL;
        return;
    }
    p->Close();
    delete p;
}

void CAppLog::Log(LOGHANDLE _handle, int _level, const char *fmt, ...)
{
    CLogHandle *p = (CLogHandle *)_handle;
    if (p == NULL) {
        if (g_loghandle == NULL)
            return;
        p = g_loghandle;
    }

    va_list args;
    va_start(args, fmt);
    p->Log(_level, fmt, args);
    va_end(args);
}



