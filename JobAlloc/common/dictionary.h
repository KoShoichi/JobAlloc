#pragma once
#ifndef _G_DICTIONARY_H
#define _G_DICTIONARY_H

#include "object.h"

/** 
*  ��������:    ���ֵ���key�����½�һ��OBJ���󣬶��������Ϊ_st_type�Ľṹ��
*  @return      �µĶ���
*/
#define DICT_NEW_OBJ(_dict_ptr, _key, _st_type)    ((_dict_ptr)->SetObj(_key, new CGObject, true, sizeof(_st_type)))

/** 
*  ��������:    ���ֵ���key�����½�һ��OBJ���󣬶��������Ϊ_st_type�Ľṹ��
*  @return      �µĽṹ��
*/
#define DICT_NEW_OBJ_VAL(_dict_ptr, _key, _st_type) ((_st_type *)(DICT_NEW_OBJ(_dict_ptr, _key, _st_type)->Value()))

/** 
*  ��������:    ֱ�ӷ����ֵ��еĽṹ��
*  @return      ֱ�ӷ����ֵ��еĽṹ�壬��������ôΪNULL
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

    //����key��Ӧ��Object
    CGObject *GetObj(const char *_key);
    CGObject *GetObj(int _key);

    /** 
    *  ��������:    ���ֵ��м���һ�����ݣ����_key���µ����½��������������ԭ������
    *  @param _key  ����
    *  @param _obj  ��ӵ���������
    *  @param _alloc_flag  �Ƿ��ʼ��OBJ���������false����ô�������������ļ���������1������ֻ��ȷ�������������ⲿ������ʱ��ô�˲���������false
    *  @param iNum  ���ڳ�ʼ������ĵ�һ������
    *  @param pData ���ڳ�ʼ������ĵڶ�������
    *  @return      �����õ�OBJ����
    *  ��ע:        ������뵽�ֵ��Ĭ�ϻ����AddRef
    */
    CGObject *SetObj(const char *_key, CGObject *_obj, bool _alloc_flag=true, int iNum=0, void *pData=NULL);
    CGObject *SetObj(int _key, CGObject *_obj, bool _alloc_flag=true, int iNum=0, void *pData=NULL);

    /** 
    *  ��������:    ����������Ԫ�ظ���
    *  @return      ������Ԫ�ظ���
    */
    int Size();

    //�������
    void Clear();

    /** 
    *  ��������:    ɾ��ָ����Ԫ��
    */
    void Delete(const char *_key);
    void Delete(int _key);


    void *Foreach(void *it_, const char *&key_, CGObject *&value_);
    //��������������������ˣ���ô���������Զ��ͷŵ�
    //���������;�����ˣ���ôѭ����β���������ͷ�
    void FreeIt(void *it_);

    /***** �̳еĽӿ� *******/
    virtual void *Value();
    virtual void *Alloc(int iNum=0, void *pData=NULL);
    virtual void Free();

protected:
    void *m_handle;
};







#endif