#include "frame_public.h"
#include "string_array.h"



CStringArray::CStringArray()
{
    
}

CStringArray::~CStringArray()
{

}

void CStringArray::push_back(const char *_str)
{
    SetObj(-1, new CStringObj(_str));
}

void CStringArray::push_back(const char *_a, const char *_b)
{
    SetObj(-1, new CStringObj(_a, _b));
}

void CStringArray::push_back(const char *_a, int _n)
{
    SetObj(-1, new CStringObj(_a, _n));
}

CStringObj * CStringArray::At(int _pos)
{
    return (CStringObj *)GetObj(_pos);
}

const char * CStringArray::at(int _pos)
{
    CStringObj *pObj = (CStringObj *)GetObj(_pos);
    if (pObj == NULL)
        return NULL;
    return pObj->c_str();
}

char * CStringArray::ConnectStr(char *buf, char delimiter)
{
    int _size = Size();
    char str_delim[2] = {0};
    str_delim[0] = delimiter;
    for(int i=0; i<_size; ++i) {
        if (i > 0)
            strcat(buf, str_delim);
        strcat(buf, at(i));
    }
    return buf;
}
