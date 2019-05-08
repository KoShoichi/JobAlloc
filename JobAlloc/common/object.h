#pragma once
#ifndef _G_OBJECT_H
#define _G_OBJECT_H

#include "sequence.h"

//读取数据操作前，计数器1和计数器2如果相等，说明当时是确定状态，没有写入操作在进行
//读取结束时，中间如果发生过数据写入，那么计数器1必定最先发生过变动
//读取不安全时会进行重复读取操作，所以读取期间只能做赋值类型的操作
#define BEGIN_GET_SAFE_OBJ(_obj)    do { \
                                        long long _end_no=_obj->GetPostNo(); \
                                        long long _begin_no=_obj->GetPrevNo(); \
                                        if (_begin_no!=_end_no) {continue;}

#define END_GET_SAFE_OBJ(_obj)          _end_no=_obj->GetPrevNo(); \
                                        if (_begin_no==_end_no) break;} \
                                     while(1);

#define BEGIN_SET_SAFE_OBJ(_obj)    do { _obj->UpdatePrevNo();
#define END_SET_SAFE_OBJ(_obj)      _obj->UpdatePostNo(); } while(0)


/** 
*  功能描述:    一般作为结构体的容器，提供索引计数功能
*  备注:        如果继承该类，那么需要注意pValue_的析构，
*               pValue默认为存储结构体，如果由CGObject析构，那么只要pValue不为空，就会free该空间
*               所以如果继承类使用了pValue，那么由CGObject析构前，应当使pValue为空
*/
class DLL_API CGObject
{
public:
    CGObject(int obj_type=0);
    virtual ~CGObject();

    int GetObjType();
    void AddRef();
    void Release();
    virtual void *Value();
    /** 
    *  功能描述:    默认用于new一个结构体
    *  @param iNum  结构体大小 
    *  @param pData 自定义参数，继承时使用，默认情况不使用
    *  @return      新建内容的指针
    *  备注:        使用Alloc应当默认使引用计数为1
    */
    virtual void *Alloc(int iNum, void *pData=NULL);
    virtual void Free();

    //多线程下，用于安全读取数据
    long long UpdatePrevNo();
    long long UpdatePostNo();
    long long GetPrevNo();
    long long GetPostNo();    

protected:
    void *pValue_;
    Sequence *pRefCount_;
    int m_ObjType;
    
    Sequence *pPrevNo_;
    Sequence *pPostNo_;
};


/** 
*  备注: 用于包装各种基本数据类型，包括各类指针等，不需要考虑内存释放的问题
*        例如CPtrObject<int> CPtrObject<double> CPtrObject<void *>
*/
template <class _T>
class CPtrObject : public CGObject
{
public:
    _T v;    
    CPtrObject(_T _src=0) {
        v = _src;
        pValue_ = &v;
    }
    virtual ~CPtrObject() {

    }
};


/** 
*  备注: 用于包装各种Class对象，之后对象的释放可以由CClassPtrObject来托管
*/
template <class _T>
class CClassPtrObject : public CGObject
{
public:
    _T *p;
    bool A_;
    CClassPtrObject() {
        p = new _T;
        A_ = true;
    }
    CClassPtrObject(_T *pSrc, bool _a=true) {
        p = pSrc;
        A_ = _a;
    }
    virtual ~CClassPtrObject() {
        if (p != NULL && A_) {
            delete p;
            p = NULL;
        }
    }
    virtual void *Value() {
        return p;
    }
};

//ProducerData结构的释放函数，调用Publish时使用
void DLL_API free_object_data(void *_data);



#endif