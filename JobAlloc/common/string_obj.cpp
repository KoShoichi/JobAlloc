#include "frame_public.h"
#include "string_obj.h"


CStringObj::CStringObj()
{
    m_size = 100;
    Alloc(m_size);
    strcpy((char *)pValue_, "");
}

CStringObj::CStringObj(const char *_str)
{
    if (_str != NULL) {
        m_size = (int)strlen(_str)+1;
        Alloc(m_size);
        strcpy((char *)pValue_, _str);
    }
    else {
        m_size = 1;
        Alloc(m_size);
        strcpy((char *)pValue_, "");
    }
}

CStringObj::CStringObj(const char *_a, const char *_b)
{
    m_size = (int)(_b - _a + 1);
    Alloc(m_size);
    strncpy((char *)pValue_, _a, m_size-1);
    ((char *)pValue_)[m_size-1] = 0;
}

CStringObj::CStringObj(const char *_a, int _n)
{
    m_size = _n + 1;
    Alloc(m_size);
    strncpy((char *)pValue_, _a, _n);
    ((char *)pValue_)[_n] = 0;
}

CStringObj::~CStringObj()
{

}

const char * CStringObj::c_str()
{
    return (const char *)pValue_;
}

char * CStringObj::val(const char *_str)
{
    if (_str == NULL) {
        ((char *)pValue_)[0] = 0;
    }
    else {
        int new_size = (int)strlen(_str)+1;
        if (new_size > m_size) {
            free(pValue_);
            pValue_ = calloc(1, new_size);
            m_size = new_size;
        }
        strcpy((char *)pValue_, _str);
    }
    return (char *)pValue_;
}

char * CStringObj::val(int _resize, bool _clear)
{
    if (_resize > m_size) {
        void *buf = calloc(1, _resize);
        m_size = _resize;
        if (_clear == false)
            strcpy((char *)buf, (const char *)pValue_);
        free(pValue_);
        pValue_ = buf;        
    }
    else {
        if (_clear == true)
            ((char *)pValue_)[0] = 0;
    }
    return (char *)pValue_;
}

char * CStringObj::val()
{
    return (char *)pValue_;
}

char * CStringObj::val(const char *_a, const char *_b)
{
    int new_size = (int)(_b - _a + 1);
    if (new_size > m_size) {
        free(pValue_);
        pValue_ = calloc(1, new_size);
        m_size = new_size;
    }
    strncpy((char *)pValue_, _a, new_size-1);
    ((char *)pValue_)[new_size-1] = 0;
    return (char *)pValue_;
}

char * CStringObj::val(const char *_a, int _n)
{
    int new_size = _n + 1;
    if (new_size > m_size) {
        free(pValue_);
        pValue_ = calloc(1, new_size);
        m_size = new_size;
    }
    strncpy((char *)pValue_, _a, _n);
    ((char *)pValue_)[_n] = 0;
    return (char *)pValue_;
}
