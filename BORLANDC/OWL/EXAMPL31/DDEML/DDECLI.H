// ObjectWindows - (C) Copyright 1992 by Borland International
//
// ddecli.h

#include <owl.h>
#include <inputdia.h>
#include <ddeml.h>
#include <stdio.h>
#include <string.h>

#include "ddeclir.h"

_CLASSDEF(TDMLClApp)
class TDMLClApp : public TApplication
{
public:
    TDMLClApp( LPSTR, HINSTANCE, HINSTANCE, LPSTR, int );
    ~TDMLClApp();
    virtual void InitMainWindow();
};

_CLASSDEF(TDMLClWnd)
class TDMLClWnd : public TWindow
{
public:
    TDMLClWnd( PTWindowsObject, LPSTR );
    virtual ~TDMLClWnd();
    virtual void SetupWindow();
    virtual void Paint( HDC, PAINTSTRUCT _FAR & );
    virtual void WMInitMenu( RTMessage ) = [WM_FIRST + WM_INITMENU];
    virtual void CMExit( RTMessage ) = [CM_FIRST + CM_EXIT];
    virtual void CMUConnect( RTMessage ) = [CM_FIRST + CM_U_CONNECT];
    virtual void CMURequest( RTMessage ) = [CM_FIRST + CM_U_REQUEST];
    virtual void CMUPoke( RTMessage ) = [CM_FIRST + CM_U_POKE];
    virtual void CMUAdvise( RTMessage ) = [CM_FIRST + CM_U_ADVISE];
    virtual void CMUHelpAbout( RTMessage ) = [CM_FIRST + CM_U_HELPABOUT];
    virtual void ReceivedData( HDDEDATA );
    static HDDEDATA FAR PASCAL _export CallBack( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );

    char szData[128];
    DWORD idInst;
    HCONV hConv;
    BOOL tfLoop;
    HSZ hszService, hszTopic, hszItem;
    FARPROC lpCallBack;
};

