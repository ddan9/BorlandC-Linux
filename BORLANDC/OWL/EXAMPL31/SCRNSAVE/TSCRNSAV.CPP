// ObjectWindows - (C) Copyright 1992 by Borland International
//
// tscrnsav.cpp

#include <owl.h>
#include "tscrnsav.h"

#ifndef SC_SCREENSAVE
#define SC_SCREENSAVE 0xF140
#endif

TScrnSavWindow::TScrnSavWindow( PTWindowsObject AParent, LPSTR ATitle,
                                PTModule AModule ) :
                TWindow( AParent, ATitle, AModule )
{
    ShowCursor( FALSE );
    Attr.Style  = WS_POPUP;
}


TScrnSavWindow::~TScrnSavWindow()
{
    ShowCursor( TRUE );
}


void TScrnSavWindow::GetWindowClass( WNDCLASS & AWndClass )
{
    TWindow::GetWindowClass( AWndClass );
    AWndClass.hIcon  = ( HICON )NULL;
    AWndClass.style |= CS_SAVEBITS;
    AWndClass.hbrBackground = ( HBRUSH )GetStockObject( NULL_BRUSH );
}


void TScrnSavWindow::SetupWindow( void )
{
    RECT rc;

    TWindow::SetupWindow();
    GetCursorPos( &prevPt );
    GetWindowRect( GetDesktopWindow(), &rc );
    MoveWindow( HWindow, rc.left, rc.top, rc.right, rc.bottom, TRUE );
}


void TScrnSavWindow::DefWndProc( RTMessage msg )
{
    switch( msg.Message )
    {
        case WM_MOUSEMOVE:
             if ( MAKEPOINT( msg.LParam ).x == prevPt.x &&
                  MAKEPOINT( msg.LParam ).y == prevPt.y  )
                 break;

        case WM_ACTIVATE:
        case WM_ACTIVATEAPP:
             if ( msg.WParam != 0 )
                 break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
             PostMessage( HWindow, WM_CLOSE, 0, 0L );

        default:
             break;
    }
    TWindow::DefWndProc( msg );
}


void TScrnSavWindow::WMSysCommand( RTMessage msg )
{
    if (( msg.WParam & 0xFFF0 ) == SC_SCREENSAVE )
    {
            msg.Result = TRUE;
    }
    else
        DefWndProc( msg );
}


void TScrnSavWindow::AnimateScreen()
{}



void TScrnSavApp::InitMainWindow()
{
    if ( *(( WORD FAR * )lpCmdLine ) == '/c' ||
         *(( WORD FAR * )lpCmdLine ) == '-c'  )
    {
        fConfigureFlag = TRUE;
        InitConfigDialog();
        if ( pConfigureDialog )
            MainWindow = pConfigureDialog;
    }
    else
    {
        fConfigureFlag = FALSE;
        InitScrnSavWindow();
        if ( pScrnSavWnd )
            MainWindow = pScrnSavWnd;
    }
}


void TScrnSavApp::InitScrnSavWindow()
{
    pScrnSavWnd = new TScrnSavWindow( NULL, NULL, NULL );
}


void TScrnSavApp::IdleAction()
{
    if ( fConfigureFlag == FALSE  &&  pScrnSavWnd != NULL )
        pScrnSavWnd->AnimateScreen();
}
