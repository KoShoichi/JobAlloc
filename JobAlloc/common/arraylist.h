#pragma once
#ifndef _G_ARRAYLIST_H
#define _G_ARRAYLIST_H

#include "object.h"


/** 
*  ��������:    �ڶ���β������һ��OBJ���󣬶��������Ϊ_st_type�Ľṹ��
*  @return      �µĶ���
*/
#define ARRAY_APEND_NEW_OBJ(_array_ptr, _st_type)    ((_array_ptr)->SetObj(-1, new CGObject, true, sizeof(_st_type)))

/** 
*  ��������:    ���ֵ���key�����½�һ��OBJ���󣬶��������Ϊ_st_type�Ľṹ��
*  @return      �µĽṹ��
*/
#define ARRAY_APPEND_NEW_OBJ_VAL(_array_ptr, _st_type) ((_st_type *)(ARRAY_APEND_NEW_OBJ(_array_ptr, _st_type)->Value()))

/** 
*  ��������:    ֱ�ӷ��ض����еĽṹ��
*  @return      ֱ�ӷ��ض����еĽṹ�壬��������ôΪNULL
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
    *  ��������:    ��ȡarray�е�����
    *  @param _index  ���������򷵻�NULL��<0���ʾ��arrayĩβ��ȡ
    *  @return      ����ָ���OBJ
    */
    CGObject *GetObj(int _index);

    /** 
    *  ��������:    ��arrayָ��λ�ò�������
    *  @param _pos  �����������ָ�����֮ǰ��0��������ڶ��ף���������Ӷ�β��ʼ�������Ҽ���ָ������֮��-1��������ڶ�β
    *  @param _obj  ��ӵ���������
    *  @param _alloc_flag  �Ƿ��ʼ��OBJ����, �����false����ô������к����ļ���������1������ֻ��ȷ�������������ⲿ������ʱ��ô�˲���������false
    *  @param iNum  ���ڳ�ʼ������ĵ�һ������
    *  @param pData ���ڳ�ʼ������ĵڶ�������
    *  @return      �����õ�OBJ����
    *  ��ע:        _pos�����߽��򷵻�NULL�����߳�д���̶߳���Ϊ�ǰ�ȫ��
    */
    CGObject *SetObj(int _pos, CGObject *_obj, bool _alloc_flag=true, int iNum=0, void *pData=NULL);

    /** 
    *  ��������:    ����������Ԫ�ظ���
    *  @return      ������Ԫ�ظ���
    */
    int Size();

    /** 
    *  ��������:    �������
    */
    void Clear();

    /** 
    *  ��������:    ����
    */
    void Sort(array_sort_call_back_t _func);

    /** 
    *  ��������:    ɾ��ָ����ŵ�Ԫ�أ�������ɾ��Ԫ�غ������Ĵ�С
    *  @param _pos  �������������ͷ����0Ϊ��׼��Ԫ����ţ�0��������ף���������Ӷ�β��ʼ������-1�������β
    *  @return      ɾ��Ԫ�غ������Ĵ�С
    */
    int Delete(int _pos);

    /***** �̳еĽӿ� *******/
    virtual void *Value();
    virtual void *Alloc(int iNum=0, void *pData=NULL);
    virtual void Free();
   

protected:
    void *m_handle;
};







#endif