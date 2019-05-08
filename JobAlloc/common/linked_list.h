#pragma once
#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

typedef void (*free_node_func_t)(void *list_node);
typedef void * NODEHANDLE;

//��ʹ��free���ͷŽڵ�
void DLL_API default_free_node_func(void *list_node);

class DLL_API CLinkedList
{
public:
    CLinkedList();
    ~CLinkedList();
    
    NODEHANDLE PreNode(NODEHANDLE pHandle);
    NODEHANDLE NextNode(NODEHANDLE pHandle);
    NODEHANDLE HeadNode();
    NODEHANDLE EndNode();

    //pHandleΪNULL���������ͷ
    NODEHANDLE InsertBefore(NODEHANDLE pHandle, void *_value, free_node_func_t _func=NULL);
    //pHandleΪNULL���������β
    NODEHANDLE InsertAfter(NODEHANDLE pHandle, void *_value, free_node_func_t _func=NULL);
    
    void EraseNode(NODEHANDLE pHandle);
    //Ҳ��ɾ���ڵ㣬���Ƿ��ؽڵ��Value������Value�����ͷ�
    void *PopNode(NODEHANDLE pHandle);
    void *GetValue(NODEHANDLE pHandle);
    
    //��ָ���Ľڵ㣬ʹ���µ������滻ԭ�����ݣ�����ԭ������
    void *UpdateValue(NODEHANDLE pHandle, void *new_value);
    
    int Size();
    void Clear();

private:
    int m_size;
    NODEHANDLE m_pHead;
    NODEHANDLE m_pEnd;
};





#endif