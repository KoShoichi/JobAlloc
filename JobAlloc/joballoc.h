
#if !defined(AFX_TEST_H__44FAEA34_0C31_4506_B711_DAACF399E091__INCLUDED_)
#define AFX_TEST_H__44FAEA34_0C31_4506_B711_DAACF399E091__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

#define WM_UPDATESCREEN WM_USER+100

extern HWND GetWndHand();
extern void SetXVal(int x);
extern void SetYVal(int y);
extern void SetSzName(char name[]);
#endif // !defined(AFX_TEST_H__44FAEA34_0C31_4506_B711_DAACF399E091__INCLUDED_)
