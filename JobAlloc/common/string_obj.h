#pragma once
#ifndef _G_STRING_OBJ_H
#define _G_STRING_OBJ_H

#include "object.h"

class DLL_API CStringObj : public CGObject
{
public:
    CStringObj();
    CStringObj(const char *_str);
    CStringObj(const char *_a, const char *_b);
    CStringObj(const char *_a, int _n);
    virtual ~CStringObj();

    const char *c_str();
    char *val(const char *_str);
    char *val(int _resize, bool _clear);
    char *val(const char *_a, const char *_b);
    char *val(const char *_a, int _n);
    char *val();

    
protected:
    int m_size;
};


#endif