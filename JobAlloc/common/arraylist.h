#pragma once
#ifndef _G_ARRAYLIST_H
#define _G_ARRAYLIST_H

#include "object.h"


/** 
*  功能描述:    在队列尾部加入一个OBJ对象，对象的内容为_st_type的结构体
*  @return      新的对象
*/
#define ARRAY_APEND_NEW_OBJ(_array_ptr, _st_type)    ((_array_ptr)->SetObj(-1, new CGObject, true, sizeof(_st_type)))

/** 
*  功能描述:    在字典中key键下新建一个OBJ对象，对象的内容为_st_type的结构体
*  @return      新的结构体
*/
#define ARRAY_APPEND_NEW_OBJ_VAL(_array_ptr, _st_type) ((_st_type *)(ARRAY_APEND_NEW_OBJ(_array_ptr, _st_type)->Value()))

/** 
*  功能描述:    直接返回队列中的结构体
*  @return      直接返回队列中的结构体，不存在那么为NULL
*/
#define ARRAY_GET_OBJ_VAL(_array_ptr, _pos, _st_type, _val)   NULL;{ CGObject *_obj = _array_ptr->GetObj(_pos); \
                                                                if (_obj) _val = (_st_type *)_obj->Value();}



typedef bool (*array_sort_call_back_t)(CGObject *i, CGObject *j);

class DLL_API CArrayList : public CGObject
{
public:
    CArrayList();
    virtual ~CArrayList();
    
    /** 
    *  功能描述:    获取array中的数据
    *  @param _index  索引超限则返回NULL，<0则表示从array末尾获取
    *  @return      索引指向的OBJ
    */
    CGObject *GetObj(int _index);

    /** 
    *  功能描述:    在array指定位置插入数据
    *  @param _pos  正数代表加在指定序号之前，0即代表加在队首，负数代表从队尾开始索引并且加在指定索引之后，-1即代表加在队尾
    *  @param _obj  添加的数据容器
    *  @param _alloc_flag  是否初始化OBJ对象, 如果是false，那么加入队列后对象的计数会增加1，所以只有确定对象早先在外部有引用时那么此参数才能是false
    *  @param iNum  用于初始化对象的第一个参数
    *  @param pData 用于初始化对象的第二个参数
    *  @return      新设置的OBJ对象
    *  备注:        _pos超出边界则返回NULL，单线程写多线程读认为是安全的
    */
    CGObject *SetObj(int _pos, CGObject *_obj, bool _alloc_flag=true, int iNum=0, void *pData=NULL);

    /** 
    *  功能描述:    返回容器内元素个数
    *  @return      容器内元素个数
    */
    int Size();

    /** 
    *  功能描述:    清空容器
    */
    void Clear();

    /** 
    *  功能描述:    排序
    */
    void Sort(array_sort_call_back_t _func);

    /** 
    *  功能描述:    删除指定序号的元素，并返回删除元素后容器的大小
    *  @param _pos  正数代表从容器头部以0为基准的元素序号，0即代表队首，负数代表从队尾开始索引，-1即代表队尾
    *  @return      删除元素后容器的大小
    */
    int Delete(int _pos);

    /***** 继承的接口 *******/
    virtual void *Value();
    virtual void *Alloc(int iNum=0, void *pData=NULL);
    virtual void Free();
   

protected:
    void *m_handle;
};







#endif