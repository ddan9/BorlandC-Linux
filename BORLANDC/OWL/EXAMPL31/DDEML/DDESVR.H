// ObjectWindows - (C) Copyright 1992 by Borland International
//
// ddesvr.h

#include <owl.h>
#include <ddeml.h>
#include <string.h>

#define CM_U_ABOUT 0x100

_CLASSDEF(TDMLSrApp)
class TDMLSrApp : public TApplication
{
public:
    TDMLSrApp( LPSTR, HINSTANCE, HINSTANCE, LPSTR, int );
    ~TDMLSrApp();
    virtual void InitMainWindow();
    virtual void IdleAction();
};

_CLASSDEF(TDMLSrWnd)
class TDMLSrWnd : public TWindow
{
public:
    TDMLSrWnd( PTWindowsObject, LPSTR );
    virtual ~TDMLSrWnd();
    virtual void SetupWindow();
    virtual void WMSysCommand( RTMessage ) = [WM_FIRST + WM_SYSCOMMAND];
    virtual void WMQueryOpen( RTMessage ) = [WM_FIRST + WM_QUERYOPEN];
    virtual BOOL MatchTopicAndService( HSZ, HSZ );
    virtual BOOL MatchTopicAndItem( HSZ, HSZ );
    virtual HDDEDATA WildConnect( HSZ, HSZ, WORD );
    virtual HDDEDATA DataRequested( WORD );
    virtual void UpdateData();
    static HDDEDATA FAR PASCAL _export CallBack( WORD, WORD, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );

    DWORD idInst;
    HCONV hConv;
    BOOL tfLoop;
    HSZ hszService, hszTopic, hszItem;
    FARPROC lpfnCallBack;
};

