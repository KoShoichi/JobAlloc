#include "TaskConsumer.h"


CTaskConsumer::CTaskConsumer()
{
    m_pFlowMap = new CDictionary;
    m_pHandlerMap = new CDictionary;
}


CTaskConsumer::~CTaskConsumer()
{    
    m_pHandlerMap->Release();
    delete m_pFlowMap;
}

int CTaskConsumer::Execute(int ret_code, int _type, void *_data, CTaskHandler *_handler, void *_context)
{
    CTaskConsumer *pNext = (CTaskConsumer *)m_pFlowMap->GetObj(ret_code);
    if (pNext != NULL) {
        if (_handler != NULL) {
            int obj_type = _handler->GetObjType();
            if (m_pHandlerMap->GetObj(obj_type) == NULL) {
                m_pHandlerMap->SetObj(obj_type, _handler, false);
            }
        }
        return pNext->Execute(ret_code, _type, _data, _handler, _context);
    }
    m_pHandlerMap->Clear();
    return 0;
}

void CTaskConsumer::SetNext(int ret_code, CTaskConsumer *_next)
{    
    CTaskConsumer *pNext = (CTaskConsumer *)m_pFlowMap->GetObj(ret_code);
    if (pNext == NULL)
        m_pFlowMap->SetObj(ret_code, _next);
    else {
        m_pFlowMap->SetObj(ret_code, _next);
        _next->SetNext(ret_code, pNext);
    }
    _next->SetHandlerMap(m_pHandlerMap);
}

void CTaskConsumer::SetHandlerMap(CDictionary *pHandlerMap)
{
    if (m_pHandlerMap == pHandlerMap)
        return;
    if (m_pHandlerMap != NULL) {
        m_pHandlerMap->Release();
    }
    m_pHandlerMap = pHandlerMap;
    m_pHandlerMap->AddRef();
    BEGIN_DICT_FOREACH(m_pFlowMap, _it, _key, _value);
    CTaskConsumer *pNext = (CTaskConsumer *)_value;
    pNext->SetHandlerMap(pHandlerMap);
    END_DICT_FOREACH(m_pFlowMap, _it);
}

CTaskHandler * CTaskConsumer::GetHandler(int handler_type, CTaskHandler *pHandlerIn)
{
	if (pHandlerIn != NULL) {
		if (pHandlerIn->GetObjType() == handler_type)
			return pHandlerIn;
	}
	return (CTaskHandler *)m_pFlowMap->GetObj(handler_type);
}

void CTaskConsumer::InsertHandler(CTaskHandler *pHandler)
{
	if (pHandler == NULL)
		return;
	int obj_type = pHandler->GetObjType();
	if (m_pHandlerMap->GetObj(obj_type) == NULL) {
		m_pHandlerMap->SetObj(obj_type, pHandler, false);
	}
}

