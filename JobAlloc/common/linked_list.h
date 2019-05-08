#pragma once
#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

typedef void (*free_node_func_t)(void *list_node);
typedef void * NODEHANDLE;

//即使用free来释放节点
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

    //pHandle为NULL则插在链表头
    NODEHANDLE InsertBefore(NODEHANDLE pHandle, void *_value, free_node_func_t _func=NULL);
    //pHandle为NULL则插在链表尾
    NODEHANDLE InsertAfter(NODEHANDLE pHandle, void *_value, free_node_func_t _func=NULL);
    
    void EraseNode(NODEHANDLE pHandle);
    //也是删除节点，但是返回节点的Value，不对Value进行释放
    void *PopNode(NODEHANDLE pHandle);
    void *GetValue(NODEHANDLE pHandle);
    
    //在指定的节点，使用新的数据替换原有数据，返回原有数据
    void *UpdateValue(NODEHANDLE pHandle, void *new_value);
    
    int Size();
    void Clear();

private:
    int m_size;
    NODEHANDLE m_pHead;
    NODEHANDLE m_pEnd;
};





#endif