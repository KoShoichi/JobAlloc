#pragma once

#include "dictionary.h"
#include "TaskHandler.h"

class CTaskConsumer : public CGObject
{
public:
    CTaskConsumer();
    virtual ~CTaskConsumer();
    virtual int Execute(int ret_code, int _type, void *_data, CTaskHandler *_handler, void *_context);
    void SetNext(int ret_code, CTaskConsumer *_next);
	CTaskHandler *GetHandler(int handler_type,  CTaskHandler *pHandlerIn);
	void InsertHandler(CTaskHandler *pHandler);

protected:
    void SetHandlerMap(CDictionary *pHandlerMap);

public:
    CDictionary *m_pFlowMap;
    CDictionary *m_pHandlerMap;
};

