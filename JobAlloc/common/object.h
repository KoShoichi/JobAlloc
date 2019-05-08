#pragma once
#ifndef _G_OBJECT_H
#define _G_OBJECT_H

#include "sequence.h"

//��ȡ���ݲ���ǰ��������1�ͼ�����2�����ȣ�˵����ʱ��ȷ��״̬��û��д������ڽ���
//��ȡ����ʱ���м��������������д�룬��ô������1�ض����ȷ������䶯
//��ȡ����ȫʱ������ظ���ȡ���������Զ�ȡ�ڼ�ֻ������ֵ���͵Ĳ���
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
*  ��������:    һ����Ϊ�ṹ����������ṩ������������
*  ��ע:        ����̳и��࣬��ô��Ҫע��pValue_��������
*               pValueĬ��Ϊ�洢�ṹ�壬�����CGObject��������ôֻҪpValue��Ϊ�գ��ͻ�free�ÿռ�
*               ��������̳���ʹ����pValue����ô��CGObject����ǰ��Ӧ��ʹpValueΪ��
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
    *  ��������:    Ĭ������newһ���ṹ��
    *  @param iNum  �ṹ���С 
    *  @param pData �Զ���������̳�ʱʹ�ã�Ĭ�������ʹ��
    *  @return      �½����ݵ�ָ��
    *  ��ע:        ʹ��AllocӦ��Ĭ��ʹ���ü���Ϊ1
    */
    virtual void *Alloc(int iNum, void *pData=NULL);
    virtual void Free();

    //���߳��£����ڰ�ȫ��ȡ����
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
*  ��ע: ���ڰ�װ���ֻ����������ͣ���������ָ��ȣ�����Ҫ�����ڴ��ͷŵ�����
*        ����CPtrObject<int> CPtrObject<double> CPtrObject<void *>
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
*  ��ע: ���ڰ�װ����Class����֮�������ͷſ�����CClassPtrObject���й�
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

//ProducerData�ṹ���ͷź���������Publishʱʹ��
void DLL_API free_object_data(void *_data);



#endif