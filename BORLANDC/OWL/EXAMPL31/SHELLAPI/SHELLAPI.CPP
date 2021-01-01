// ObjectWindows - (C) Copyright 1992 by Borland International
//
// shellapi.cpp

#include <owl.h>
#include <shellapi.h>
#include <stdio.h>
#include <dir.h>
#include <string.h>

class TMyApp : public TApplication
{

public:
    TMyApp(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance,
            LPSTR lpCmdLine, int nCmdShow)
        : TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow)
    {
    };

  virtual void InitMainWindow();

};

_CLASSDEF(TDropTargetWin)
class TDropTargetWin : public TWindow
{

public:
  TDropTargetWin(PTWindowsObject AParent, LPSTR ATitle)
    : TWindow(AParent, ATitle)
  {
  };

protected:

  virtual void SetupWindow();

  virtual void WMDropFiles(RTMessage Msg) = [ WM_FIRST + WM_DROPFILES ];

    // Override this function in derived classes to change behavior:

  virtual void DropAFile(char *fileName, int x, int y );

};

_CLASSDEF(TIconWindow)
class TIconWindow : public TWindow
{
public:
    TIconWindow(PTWindowsObject AParent, LPSTR ATitle, int x, int y);
    ~TIconWindow();

protected:
    virtual void WMQueryDragIcon(RTMessage) = [ WM_FIRST + WM_QUERYDRAGICON ];
    virtual void WMQueryOpen    (RTMessage) = [ WM_FIRST + WM_QUERYOPEN ];
    virtual void WMSysCommand   (RTMessage) = [ WM_FIRST + WM_SYSCOMMAND ];

    virtual void  Paint( HDC, PAINTSTRUCT& );
    virtual void  GetWindowClass(WNDCLASS _FAR &);
    virtual LPSTR GetClassName();

private:
    HICON   hIcon;
    BOOL    bDefIcon;
    LPSTR	pPath;
    int     x,y;

};

//---------------------------------------------------------------
// ---- Code starts here -----
//---------------------------------------------------------------


TIconWindow::TIconWindow(PTWindowsObject AParent, LPSTR ATitle, int _x, int _y)
    : TWindow(AParent,"unknown")
{

    Attr.Style |= (WS_MINIMIZE | WS_CHILD);

    // Set Title to file name

    char    fn[MAXFILE];
    char	pTemp[255];

    _fstrcpy(pTemp,ATitle);

    *fn = 0;

    fnsplit( pTemp, NULL, NULL, fn, NULL );

    if( *fn )
    {
    	delete Title;
        Title = _fstrdup(fn);
    }

    // Set pPath to full path for execution

    pPath = _fstrdup(ATitle);


    // Get icon from executable

    HANDLE h = FindExecutable( pPath, ".\\", pTemp );

    if( WORD(h) <= 32 )
        hIcon = 0;
    else
        hIcon = ExtractIcon(GetModule()->hInstance,pTemp,0);

    if( WORD(hIcon) <= 1 )
    {
        hIcon = LoadIcon( NULL, IDI_QUESTION );
        bDefIcon = 1;
    }
    else
    {
        bDefIcon = 0;
    }

    // Set x/y position of drop (in Parent coordinates - not used in this app)

    x = _x;
    y = _y;

}

TIconWindow::~TIconWindow()
{
    if( bDefIcon )
    {
        // Don't call FreeResource() on the '?' icon

        FreeResource(hIcon);
    }

    delete pPath;
}

void TIconWindow::GetWindowClass(WNDCLASS _FAR & wc)
{
    TWindow::GetWindowClass(wc);

    // If we don't set our class's icon to NULL, USER won't let us
    // paint our own...

    wc.hIcon = NULL;
}

LPSTR TIconWindow::GetClassName()
{
    return( "TIconWindow" );
}

void TIconWindow::WMQueryOpen(RTMessage Msg)
{
    ShellExecute
        (
            HWindow,
            NULL,
            pPath,
            "",
            ".\\",
            SW_SHOWNORMAL
         );

    Msg.Result = 0L;
}

void TIconWindow::WMQueryDragIcon(RTMessage Msg)
{
    Msg.Result = DWORD(hIcon);
}

void TIconWindow::WMSysCommand(RTMessage Msg)
{
        // Capturing these variations on WM_SYSCOMMAND prevent an
        // annoying 'beep' on single clicks on the icon

	switch(Msg.WParam & 0xFFF0)
	{
		case SC_MOUSEMENU:
        case SC_KEYMENU:
        	Msg.Result = 0L;
            break;
        default:
        	DefWndProc(Msg);
    }
}

void TIconWindow::Paint(HDC hDC, PAINTSTRUCT& )
{
        // This is the 'new' way to draw you own icon, as opposed
        // to WM_PAINTICON in Win3.0

    DefWindowProc(HWindow, WM_ICONERASEBKGND, WORD(hDC), 0L );
    DrawIcon(hDC, 0, 0, hIcon );
}

void TDropTargetWin::SetupWindow()
{
    TWindow::SetupWindow();

        // Let Shell.dll flip the WS_EX_ACCEPTFILES style bit
        // for this window

    DragAcceptFiles(HWindow,TRUE);
}

void TDropTargetWin::WMDropFiles(RTMessage Msg)
{
    POINT   ptDrop;
    HDROP   hDrop = HDROP(Msg.WParam);

    DragQueryPoint( hDrop, &ptDrop );

        // By passing in exactly these parameters, we get the
        // number of files (and directories) being dropped

    int nNumDropped = DragQueryFile( hDrop, -1U, NULL, 0 );

    char *p = new char[255];

    for( int i = 0; i < nNumDropped; i++)
    {
        // This time we pass in the 'real' parameters and SHELL.DLL
        // will fill in the path to the file (or directory)

        DragQueryFile( hDrop, i, p, 255 );

        DropAFile( p, ptDrop.x, ptDrop.y );
    }

    delete p;

    DragFinish( hDrop );
}

void TDropTargetWin::DropAFile( char *fileName, int x, int y)
{
    PTIconWindow pIW = new TIconWindow( this, fileName, x, y );

    GetApplication()->MakeWindow(pIW);
}


void TMyApp::InitMainWindow()
{
  MainWindow = new TDropTargetWin(NULL, Name);
}


int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpCmdLine, int nCmdShow)
{
  TMyApp MyApp("Sample ObjectWindows Program/Shell API",
                hInstance, hPrevInstance,
                lpCmdLine, nCmdShow);
  MyApp.Run();
  return MyApp.Status;
}
