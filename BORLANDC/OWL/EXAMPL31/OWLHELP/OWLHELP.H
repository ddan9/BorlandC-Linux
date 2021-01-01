// ObjectWindows - (C) Copyright 1992 by Borland International
//
// owlhelp.h

#ifndef __OWL_H
#include <owl.h>
#endif

_CLASSDEF(TOWLHELPApp)
class TOWLHELPApp : public TApplication
{
public:
    TOWLHELPApp( LPSTR, HINSTANCE, HINSTANCE, LPSTR, int );
    virtual void InitInstance();
    virtual void InitMainWindow();
};

_CLASSDEF(TOWLHELPWnd)
class TOWLHELPWnd : public TWindow
{
public:
    TOWLHELPWnd( PTWindowsObject, LPSTR );
	virtual void GetWindowClass( WNDCLASS & );
    virtual void Paint( HDC, PAINTSTRUCT _FAR &);
    virtual LPSTR GetClassName();
    virtual void SetupWindow();
    virtual void WMEnterIdle( RTMessage ) = [WM_FIRST + WM_ENTERIDLE];
	virtual void CMUMenuItemA( RTMessage ) = [CM_FIRST + CM_U_MENUITEMA];
    virtual void CMUMenuItemB( RTMessage ) = [CM_FIRST + CM_U_MENUITEMB];
    virtual void CMExit( RTMessage ) = [CM_FIRST + CM_EXIT];
    virtual void CMUHelpIndex( RTMessage ) = [CM_FIRST + CM_U_HELPINDEX];
    virtual void CMUHelpHelp( RTMessage ) = [CM_FIRST + CM_U_HELPHELP];
    virtual void CMUHelpAbout( RTMessage ) = [CM_FIRST + CM_U_HELPABOUT];

    BOOL tfF1Pressed;
};

