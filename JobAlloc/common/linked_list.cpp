#include "frame_public.h"
#include "linked_list.h"
#include "applog.h"


typedef struct _list_node_t
{
    _list_node_t  *pPrev;
    _list_node_t  *pNext;
    void *pValue;
    free_node_func_t pFree;
}list_node_t;

void DLL_API default_free_node_func(void *list_node)
{
    free(list_node);
}

CLinkedList::CLinkedList()
{
    m_size = 0;
    m_pHead = m_pEnd = NULL;
}

CLinkedList::~CLinkedList()
{
    Clear();
}

NODEHANDLE CLinkedList::PreNode(NODEHANDLE pHandle)
{
    if (pHandle==NULL || m_size<=0)
        return NULL;
    return ((list_node_t *)pHandle)->pPrev;
}

NODEHANDLE CLinkedList::NextNode(NODEHANDLE pHandle)
{
    if (pHandle==NULL || m_size<=0)
        return NULL;
    return ((list_node_t *)pHandle)->pNext;
}

NODEHANDLE CLinkedList::HeadNode()
{
    if (m_size <= 0)
        return NULL;
    else
        return m_pHead;
}

NODEHANDLE CLinkedList::EndNode()
{
    if (m_size <= 0)   
        return NULL;
    else
        return m_pEnd;
}

NODEHANDLE CLinkedList::InsertBefore(NODEHANDLE pHandle, void *_value, free_node_func_t _func/*=NULL*/)
{
    list_node_t *it = (list_node_t *)calloc(1, sizeof(list_node_t));
    if (it == NULL)
        return NULL;

    it->pValue = _value;
    it->pFree = _func;

    if (pHandle == NULL)
        pHandle = m_pHead;

    list_node_t *p = (list_node_t *)pHandle;
    if ((it->pNext=p) == NULL) {
        it->pPrev = (list_node_t *)m_pEnd;
        m_pEnd = (NODEHANDLE)it;
    }
    else {
        it->pPrev = p->pPrev;
        p->pPrev = it;
    }

    if (it->pPrev == NULL)
        m_pHead = (NODEHANDLE)it;
    else
        it->pPrev->pNext = it;

    ++m_size;
    return (NODEHANDLE)it;
}

NODEHANDLE CLinkedList::InsertAfter(NODEHANDLE pHandle, void *_value, free_node_func_t _func/*=NULL*/)
{
    list_node_t *it = (list_node_t *)calloc(1, sizeof(list_node_t));
    if (it == NULL)
        return NULL;

    it->pValue = _value;
    it->pFree = _func;

    if (pHandle == NULL)
        pHandle = m_pEnd;

    list_node_t *p = (list_node_t *)pHandle;
    if ((it->pPrev=p) == NULL) {
        it->pNext = (list_node_t *)m_pHead;
        m_pHead = (NODEHANDLE)it;
    }
    else {
        it->pNext = p->pNext;
        p->pNext = it;
    }

    if (it->pNext==NULL)
        m_pEnd = (NODEHANDLE)it;
    else
        it->pNext->pPrev = it;
    
    ++m_size;
    //GLOG_DEBUG("InsertAfter处理 地址:0x%08x 后:0x%08x 前:0x%08x", it, it->pNext, it->pPrev);
    return (NODEHANDLE)it;
}

void CLinkedList::EraseNode(NODEHANDLE pHandle)
{
    if (pHandle==NULL || m_size<=0)
        return;
    
    --m_size;
    list_node_t *p = (list_node_t *)pHandle;
    if (p->pNext == NULL)
        m_pEnd = (NODEHANDLE)p->pPrev;
    else
        p->pNext->pPrev = p->pPrev;
    
    if (p->pPrev == NULL)
        m_pHead = (NODEHANDLE)p->pNext;
    else
        p->pPrev->pNext = p->pNext;

    p->pPrev = p->pNext = NULL;
    if (p->pFree != NULL && p->pValue != NULL) {
        p->pFree(p->pValue);
        p->pValue=NULL;
    }
    free(p);
}

void * CLinkedList::PopNode(NODEHANDLE pHandle)
{
    if (pHandle==NULL || m_size<=0)
        return NULL;

    --m_size;
    list_node_t *p = (list_node_t *)pHandle;
    if (p->pNext == NULL)
        m_pEnd = (NODEHANDLE)p->pPrev;
    else
        p->pNext->pPrev = p->pPrev;

    if (p->pPrev == NULL)
        m_pHead = (NODEHANDLE)p->pNext;
    else
        p->pPrev->pNext = p->pNext;

    p->pPrev = p->pNext = NULL;
    void *pValue = p->pValue;
    free(p);
    return pValue;
}


void * CLinkedList::GetValue(NODEHANDLE pHandle)
{
    if (pHandle == NULL)
        return NULL;
    return ((list_node_t *)pHandle)->pValue;
}

int CLinkedList::Size()
{
    return m_size;
}

void CLinkedList::Clear()
{
    list_node_t *pHead = (list_node_t *)m_pHead;
    list_node_t *pNext = pHead;
    m_size = 0;
    m_pHead = m_pEnd = NULL;
    while (pHead != NULL)
    {
        pNext = pHead->pNext;
        if (pHead->pFree != NULL) {
            pHead->pFree(pHead->pValue);
            pHead->pValue = NULL;
        }
        free(pHead);
        pHead = pNext;
    }
}

void * CLinkedList::UpdateValue(NODEHANDLE pHandle, void *new_value)
{
    list_node_t *p = (list_node_t *)pHandle;
    void *_old = p->pValue;
    p->pValue = new_value;
    return _old;
}


