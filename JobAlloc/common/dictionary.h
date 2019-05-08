#pragma once
#ifndef _G_DICTIONARY_H
#define _G_DICTIONARY_H

#include "object.h"

/** 
*  功能描述:    在字典中key键下新建一个OBJ对象，对象的内容为_st_type的结构体
*  @return      新的对象
*/
#define DICT_NEW_OBJ(_dict_ptr, _key, _st_type)    ((_dict_ptr)->SetObj(_key, new CGObject, true, sizeof(_st_type)))

/** 
*  功能描述:    在字典中key键下新建一个OBJ对象，对象的内容为_st_type的结构体
*  @return      新的结构体
*/
#define DICT_NEW_OBJ_VAL(_dict_ptr, _key, _st_type) ((_st_type *)(DICT_NEW_OBJ(_dict_ptr, _key, _st_type)->Value()))

/** 
*  功能描述:    直接返回字典中的结构体
*  @return      直接返回字典中的结构体，不存在那么为NULL
*/
#define DICT_GET_OBJ_VAL(_dict_ptr, _key, _st_type, _val)   NULL;{ CGObject *_obj = _dict_ptr->GetObj(_key); \
                                                                 if (_obj) _val = (_st_type *)_obj->Value();}

#define BEGIN_DICT_FOREACH(_dict_ptr, _it, _key, _value)     {void* _it =NULL;const char* _key =NULL;CGObject* _value =NULL; \
                                                        while(_it = _dict_ptr->Foreach(_it, _key, _value)) {
#define END_DICT_FOREACH(_dict_ptr, _it)        } if (_it) _dict_ptr->FreeIt(_it);}


class DLL_API CDictionary : public CGObject
{
public:
    CDictionary();
    virtual ~CDictionary();

    //返回key对应的Object
    CGObject *GetObj(const char *_key);
    CGObject *GetObj(int _key);

    /** 
    *  功能描述:    往字典中加入一项数据，如果_key是新的则新建主键，否则替代原有主键
    *  @param _key  主键
    *  @param _obj  添加的数据容器
    *  @param _alloc_flag  是否初始化OBJ对象，如果是false，那么加入容器后对象的计数会增加1，所以只有确定对象早先在外部有引用时那么此参数才能是false
    *  @param iNum  用于初始化对象的第一个参数
    *  @param pData 用于初始化对象的第二个参数
    *  @return      新设置的OBJ对象
    *  备注:        对象加入到字典后，默认会调用AddRef
    */
    CGObject *SetObj(const char *_key, CGObject *_obj, bool _alloc_flag=true, int iNum=0, void *pData=NULL);
    CGObject *SetObj(int _key, CGObject *_obj, bool _alloc_flag=true, int iNum=0, void *pData=NULL);

    /** 
    *  功能描述:    返回容器内元素个数
    *  @return      容器内元素个数
    */
    int Size();

    //清空容器
    void Clear();

    /** 
    *  功能描述:    删除指定的元素
    */
    void Delete(const char *_key);
    void Delete(int _key);


    void *Foreach(void *it_, const char *&key_, CGObject *&value_);
    //如果把整个容器都遍历了，那么迭代器是自动释放的
    //如果遍历中途跳出了，那么循环结尾处来调用释放
    void FreeIt(void *it_);

    /***** 继承的接口 *******/
    virtual void *Value();
    virtual void *Alloc(int iNum=0, void *pData=NULL);
    virtual void Free();

protected:
    void *m_handle;
};







#endif