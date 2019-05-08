#include "frame_public.h"
#include "dictionary.h"

//���ֱ�Ӱ�STL���ͷ���ͷ�ļ��ж����˳�Ա��������ôDLL����ʱ�����warning C4251��������Ըô����ڲ�ͬ�����������Ӹ�dll���ǿ��ܴ�������
//���԰Ѵ����ݰ���cpp�б��룬�ǰ�ȫ��
typedef std::map< std::string, CGObject * > map_t;

CDictionary::CDictionary()
{
    m_handle = (void *)new map_t;
}

CDictionary::~CDictionary()
{
    map_t *pMap = (map_t *)m_handle;
    if (pMap != NULL) {
        for (map_t::iterator it=pMap->begin(); it!=pMap->end(); ++it) {
            it->second->Release();
        }
        delete pMap;
        m_handle = NULL;
    }
}

CGObject * CDictionary::SetObj(const char *_key, CGObject *_obj, bool _alloc_flag/*=true*/, int iNum/*=0*/, void *pData/*=NULL*/)
{
    map_t *pMap = (map_t *)m_handle;
    //ָ������ã���������ֱ�ӶԷ���ֵ���и�ֵ����ô�������CObject�ĵ�ַ��ֱ�Ӹ��µ�map��
    CGObject *&pObj = (*pMap)[_key];
    if (pObj == _obj) {
        return pObj;
    }
    if (pObj != NULL)
        pObj->Release();
    pObj = _obj;
    if (_alloc_flag)
        pObj->Alloc(iNum, pData);    //Alloc���Ƿ���ռ�������⣬����ǿ�ư�������Ϊ1������������������µ�ObjectĬ��ʹ��Alloc
    else
        pObj->AddRef();
    return pObj;
}

CGObject * CDictionary::SetObj(int _key, CGObject *_obj, bool _alloc_flag/*=true*/, int iNum/*=0*/, void *pData/*=NULL*/)
{
    char buf[20] = {0};
    sprintf(buf, "%d", _key);
    return SetObj(buf, _obj, _alloc_flag, iNum, pData);
}

CGObject * CDictionary::GetObj(const char *_key)
{
    map_t *pMap = (map_t *)m_handle;
    map_t::iterator it = pMap->find(_key);
    if (it == pMap->end())
        return NULL;
    return it->second;
}

CGObject * CDictionary::GetObj(int _key)
{
    char buf[20] = {0};
    sprintf(buf, "%d", _key);
    return GetObj(buf);
}

void CDictionary::Clear()
{
    map_t *pMap = (map_t *)m_handle;
    if (pMap != NULL) {
        for (map_t::iterator it=pMap->begin(); it!=pMap->end(); ++it) {
            it->second->Release();
        }
        pMap->clear();        
    }
}


void * CDictionary::Value()
{
    return this;
}

void * CDictionary::Alloc(int iNum/*=0*/, void *pData/*=NULL*/)
{
    pRefCount_->Set(1);
    return this;
}

void CDictionary::Free()
{
    //Ĭ�ϵ�OBJ���ͷ�������������ݳ�Ա���˴�û����Ҫ�ͷŵĶ���
    return;
}

void * CDictionary::Foreach(void *it_, const char *&key_, CGObject *&value_)
{
    map_t::iterator *pit = (map_t::iterator *)it_;
    map_t *pMap = (map_t *)m_handle;
    if (pit == NULL) {
        pit = new map_t::iterator;
        *pit = pMap->begin();
        if (*pit == pMap->end()) {
            key_ = NULL;
            value_ = NULL;
            return NULL;
        }
        key_ = (*pit)->first.c_str();
        value_ = (*pit)->second;
        return pit;
    }
    if ((++(*pit)) == pMap->end()) {
        delete pit;
        key_ = NULL;
        value_ = NULL;
        return NULL;
    }
    key_ = (*pit)->first.c_str();
    value_ = (*pit)->second;
    return pit;
}

void CDictionary::FreeIt(void *it_)
{
    map_t::iterator *pit = (map_t::iterator *)it_;
    if (pit != NULL)
        delete pit;
}

void CDictionary::Delete(const char *_key)
{
    map_t *pMap = (map_t *)m_handle;
    map_t::iterator it = pMap->find(_key);
    if (it == pMap->end())
        return;
    it->second->Release();
    pMap->erase(it);
}

void CDictionary::Delete(int _key)
{
    char buf[20] = {0};
    sprintf(buf, "%d", _key);
    Delete(buf);
}

int CDictionary::Size()
{
    map_t *pMap = (map_t *)m_handle;
    return (int)pMap->size();
}

