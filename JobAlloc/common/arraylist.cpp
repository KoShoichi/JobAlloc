#include "frame_public.h"
#include <algorithm>
#include "arraylist.h"


//���ֱ�Ӱ�STL���ͷ���ͷ�ļ��ж����˳�Ա��������ôDLL����ʱ�����warning C4251��������Ըô����ڲ�ͬ�����������Ӹ�dll���ǿ��ܴ�������
//���԰Ѵ����ݰ���cpp�б��룬�ǰ�ȫ��
typedef std::deque< CGObject * > array_t;

CArrayList::CArrayList()
{
    m_handle = (void *)new array_t;
}

CArrayList::~CArrayList()
{
    array_t *pArray = (array_t *)m_handle;
    Clear();
    if (pArray != NULL) {
        delete pArray;
        m_handle = NULL;
    }
}

CGObject * CArrayList::GetObj(int _index)
{
    array_t *pArray = (array_t *)m_handle;
    int _size = (int)pArray->size();
    if (_index >= _size || _index<-_size)
        return NULL;
    if (_index < 0)
        _index = _size+_index;
    return pArray->at(_index);
}

CGObject * CArrayList::SetObj(int _pos, CGObject *_obj, bool _alloc_flag/*=true*/, int iNum/*=0*/, void *pData/*=NULL*/)
{
    array_t *pArray = (array_t *)m_handle;
    if (_pos == -1)
        pArray->push_back(_obj);
    else if (_pos == 0)
        pArray->push_front(_obj);
    else {
        int _size = (int)pArray->size();
        if (_pos >= _size) {
            _size = _pos + 1;
            pArray->resize(_size, NULL);
        }
        else if (_pos<-_size)
            return NULL;
        //������pArray�϶������㹻��Ԫ����
        array_t::iterator it_begin = pArray->begin();
        if (_pos < 0)
            _pos = _size+_pos;
        pArray->insert(it_begin+_pos, _obj);
    }
    if (_alloc_flag)
        _obj->Alloc(iNum, pData);   //Alloc��Ĭ�ϻ����AddRef
    else
        _obj->AddRef();
    return _obj;
}

int CArrayList::Size()
{
    array_t *pArray = (array_t *)m_handle;
    return (int)pArray->size();
}

void CArrayList::Clear()
{
    array_t *pArray = (array_t *)m_handle;
    if (pArray != NULL) {
        for (array_t::iterator it=pArray->begin(); it!=pArray->end(); ++it) {
            (*it)->Release();
        }
        pArray->clear();
    }
}

void * CArrayList::Value()
{
    return this;
}

void * CArrayList::Alloc(int iNum/*=0*/, void *pData/*=NULL*/)
{
    pRefCount_->Set(1);
    return this;
}

void CArrayList::Free()
{
    return;
}

void CArrayList::Sort(array_sort_call_back_t _func)
{
    array_t *pArray = (array_t *)m_handle;
    std::sort(pArray->begin(), pArray->end(), _func);
}

int CArrayList::Delete(int _pos)
{
    array_t *pArray = (array_t *)m_handle;
    int _size = (int)pArray->size();
    if (_pos >= _size || _pos<-_size)
        return -1;
    //������pArray�϶����ٰ���һ��Ԫ����
    array_t::iterator it_begin = pArray->begin();
    if (_pos < 0)
        _pos = _size+_pos;
    pArray->erase(it_begin+_pos);
    return (int)pArray->size();
}



