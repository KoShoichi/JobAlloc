#ifndef _G_APPLOG_H
#define _G_APPLOG_H

#include "frame_public.h"

typedef void * LOGHANDLE;


#define GLOG_ERROR(...)          CAppLog::Log(NULL, LOG_ERROR_T,  __VA_ARGS__)
#define GLOG_WARNING(...)        CAppLog::Log(NULL, LOG_WARNING_T,  __VA_ARGS__)
#define GLOG_MSG(...)            CAppLog::Log(NULL, LOG_MSG_T,  __VA_ARGS__)
#define GLOG_DEBUG(...)          CAppLog::Log(NULL, LOG_DEBUG_T,  __VA_ARGS__)
#define GLOG_STD(...)            CAppLog::Log(NULL, LOG_STD_T,  __VA_ARGS__)
#define GLOG_PT(...)             CAppLog::Log(NULL, LOG_PT_T,  __VA_ARGS__)


//数值越小代表重要性越高
enum LOGTYPE
{
    LOG_PT_T        = 1,		//print to screen only
    LOG_STD_T       = 10,		//print to screen && write to file    
	LOG_ERROR_T     = 100,
	LOG_WARNING_T   = 200,
	LOG_MSG_T       = 400,
	LOG_DEBUG_T     = 500	
};

class DLL_API CAppLog
{
public:
	static void      CreateGlobalHandle(const char *log_name, int log_level, unsigned int max_threads=1);
    static LOGHANDLE CreateLogHandle(const char *log_name, int log_level, unsigned int max_threads=1);

	
    static int Open(LOGHANDLE _handle = NULL);
    //LOGHANDLE关闭后就无法使用了，需要重新Create
    static void Close(LOGHANDLE _handle = NULL);

	static void Log(LOGHANDLE _handle, int _level, const char *fmt, ...);	
};

#endif