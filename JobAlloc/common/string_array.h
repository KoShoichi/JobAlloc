#pragma once
#ifndef _G_STRING_ARRAY_H
#define _G_STRING_ARRAY_H

#include "arraylist.h"
#include "string_obj.h"

class DLL_API CStringArray : public CArrayList
{
public:
    CStringArray();
    virtual ~CStringArray();

    void push_back(const char *_str);
    void push_back(const char *_a, const char *_b);
    void push_back(const char *_a, int _n);
    const char *at(int _pos);
    CStringObj *At(int _pos);
    char *ConnectStr(char *buf, char delimiter);

};


#endif