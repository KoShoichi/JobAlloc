#include "frame_public.h"
#include "dictionary.h"

//如果直接把STL类型放在头文件中定义了成员变量，那么DLL导出时会出现warning C4251，如果忽略该错误，在不同编译器中链接该dll还是可能存在问题
//所以把此内容包到cpp中编译，是安全的
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
    //指针的引用，即调用者直接对返回值进行赋值，那么所赋予的CObject的地址会直接更新到map中
    CGObject *&pObj = (*pMap)[_key];
    if (pObj == _obj) {
        return pObj;
    }
    if (pObj != NULL)
        pObj->Release();
    pObj = _obj;
    if (_alloc_flag)
        pObj->Alloc(iNum, pData);    //Alloc除非分配空间的作用外，还会强制把引用置为1，所以在容器中添加新的Object默认使用Alloc
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
    //默认的OBJ会释放自身包含的数据成员，此处没有需要释放的对象
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

