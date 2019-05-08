#pragma once
#ifndef _PATTERN_NPNC_H
#define _PATTERN_NPNC_H

#include "frame_public.h"

typedef void * HANDLE_NPNC;
typedef void (*free_data_func_t)(void *_data);
typedef int (*consumer_call_back_t)(int _type, void *_data, void *_handler, void *_context);

class DLL_API CProducer_NPNC
{
public:
    CProducer_NPNC(unsigned int max_consumer=1);
    ~CProducer_NPNC();
    int RegisterNewConsumer(consumer_call_back_t _func, void *_callback_context=NULL);
    int Publish(void *_data, int _type=0, free_data_func_t _func=NULL, void *_handler=NULL);
    HANDLE_NPNC PublishSync(void *_data, int _type=0, free_data_func_t _func=NULL, void *_handler=NULL);
    int WaitHandle(HANDLE_NPNC _handle);
    void CloseHandle(HANDLE_NPNC _handle);
    void WaitFinish();
    int GetConsumerCount();
    HANDLE_NPNC GetContextHandle();

private:
    HANDLE_NPNC m_context;
};

class DLL_API CConsumerBase
{
public:
    CConsumerBase(CConsumerBase *_next=NULL);
    virtual ~CConsumerBase();
    virtual int Execute(int _type, void *_data, void *_handler, void *_context);
    void SetNext(CConsumerBase *_next);
protected:
    CConsumerBase *m_pNext;
};





#endif