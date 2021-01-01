// ObjectWindows - (C) Copyright 1992 by Borland International
//
// cmdlgap.h

#include <owl.h>
#include <dialog.h>
#include <stdio.h>
#include <string.h>
#include <commdlg.h>
#include "cmdlgapr.h"

_CLASSDEF(TCMDLGApp)
class TCMDLGApp : public TApplication
{
public:
	TCMDLGApp( LPSTR, HINSTANCE, HINSTANCE, LPSTR, int );
	~TCMDLGApp();
	virtual void InitMainWindow();
};

_CLASSDEF(TCMDLGWnd)
class TCMDLGWnd : public TWindow
{
public:
	TCMDLGWnd( PTWindowsObject, LPSTR );
	virtual ~TCMDLGWnd();
	virtual LPSTR GetClassName();
	virtual void Paint( HDC, PAINTSTRUCT _FAR & );
	virtual void CMExit( RTMessage ) = [CM_FIRST + CM_EXIT];
	virtual void CMUFileOpen( RTMessage ) = [CM_FIRST + CM_U_FILEOPEN];
	virtual void CMUColor( RTMessage ) = [CM_FIRST + CM_U_COLOR];
	virtual void CMUFont( RTMessage ) = [CM_FIRST + CM_U_FONT];
	virtual void CMUHelpAbout( RTMessage ) = [CM_FIRST + CM_U_HELPABOUT];

	char szName[256];
	COLORREF crColor;
	HFONT hfFont;
	BOOL tfFontLoaded;
};

// end of file cmdlgap.h
