// Test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "frame_public.h"
#include "applog.h"
#include "DataContext.h"
#include "arraylist.h"
#include "employee.h"
#include "joballoc.h"

#define MAX_LOADSTRING 100

static int  xVal=200;
static int  yVal=30;

bool init = false;
char szName[] ="INITIALIZATION:";
// Global Variables:
HINSTANCE hInst;								            // current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];						// The title bar text
HWND  hResult;
HWND  hResultMid;
HWND  hTitle;
HWND  m_hwnd;
HINSTANCE m_hInstance;
// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void SetXVal(int x){
	xVal=x;
}

void SetYVal(int y){
	yVal= y;
}

void SetSzName(char name[]){
	strcpy(szName,name);
}

HWND GetWndHand(){
	return m_hwnd;
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG    msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}
	m_hInstance =hInstance;
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_TEST);

	//初始化数据上下文内容
	CAppLog::CreateGlobalHandle("Job.log", LOG_DEBUG_T, 100);
	CAppLog::Open();
	if (CDataContext::GetInstance()->Initialize() != 0)
		return -1;
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_TEST);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground =HBRUSH(GetStockObject(WHITE_BRUSH));
	wcex.lpszMenuName	= (LPCSTR)IDC_TEST;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 300, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   m_hwnd=hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	char sztail[] ="EMPLOYEE COST";
	char szTailLine[255]={0};
	char msgbuf[2048] = {0};
	CDataContext *pDataContext = CDataContext::GetInstance();
	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;

		case WM_CREATE:
			hdc=GetDC(hWnd);
			RECT Rect;
			GetClientRect(hWnd,&Rect);
			hTitle=CreateWindow("static",NULL,WS_CHILD|WS_VISIBLE|WS_BORDER,9,10,200,20,hWnd,HMENU(-1),m_hInstance,NULL);
			hResult=CreateWindow("ListBox",NULL,WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_SIZEBOX|WS_HSCROLL|LBS_NOTIFY,9,35,Rect.right/2,Rect.bottom-50,hWnd,HMENU(-1),m_hInstance,NULL);
			hResultMid=CreateWindow("ListBox",NULL,WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_SIZEBOX|WS_HSCROLL|LBS_NOTIFY,9+Rect.right/2,35,Rect.right/2-50,Rect.bottom-50,hWnd,HMENU(-1),m_hInstance,NULL);

			ListBox_SetHorizontalExtent(::hResult,5000L);
			ListBox_SetHorizontalExtent(::hResultMid,5000L);
			//ListBox_ResetContent(hResultMid);
			ReleaseDC(hWnd,hdc);
			break;
		case WM_UPDATESCREEN:
			Static_SetText(::hTitle,szName);
			ListBox_ResetContent(hResult);

			if (0==strcmp(szName,"OTNSWITCH END:"))
			{
				BEGIN_DICT_FOREACH(pDataContext->m_result, _it, _key, _value);
				memset(msgbuf,0,sizeof(msgbuf));
				CArrayList* arrayList = (CArrayList*)_value;
				strcat(msgbuf,_key);
				strcat(msgbuf,":  ");
				for (int i=0;i<arrayList->Size();++i){
					CStringValue* str =(CStringValue*)arrayList->GetObj(i);
					strcat(msgbuf,str->s_value.c_str());
					strcat(msgbuf,"->");
				}
				ListBox_AddString(hResultMid,msgbuf);
				END_DICT_FOREACH(pDataContext->m_result, _it);
				sprintf(szTailLine,"%s=%f",sztail,pDataContext->m_totalcost);
				ListBox_AddString(hResultMid,szTailLine);
			}
			

			BEGIN_DICT_FOREACH(pDataContext->m_result, _it, _key, _value);
			memset(msgbuf,0,sizeof(msgbuf));
			CArrayList* arrayList = (CArrayList*)_value;
			strcat(msgbuf,_key);
			strcat(msgbuf,":  ");
			for (int i=0;i<arrayList->Size();++i){
				CStringValue* str =(CStringValue*)arrayList->GetObj(i);
				strcat(msgbuf,str->s_value.c_str());
				strcat(msgbuf,"->");
			}
			ListBox_AddString(hResult,msgbuf);
			END_DICT_FOREACH(pDataContext->m_result, _it);
			sprintf(szTailLine,"%s=%f",sztail,pDataContext->m_totalcost);
			ListBox_AddString(hResult,szTailLine);
			break;
		case WM_DESTROY:
			CAppLog::Close();  //退出时关闭日志流文件
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
