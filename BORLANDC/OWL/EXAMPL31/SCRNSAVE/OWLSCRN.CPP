// ObjectWindows - (C) Copyright 1992 by Borland International
//
// owlscrn.cpp

#include <owl.h>
#include <dialog.h>
#include <bwcc.h>
#include <time.h>
#include "tscrnsav.h"

char  szAppName[]  = "ScreenSaver.OWLSample";
const WORD NUMDOTS =  900;

struct  DOTS
{
    POINT  ptVar;
    DWORD  dwColor;
};


_CLASSDEF( TMyScrnSavWindow )
class TMyScrnSavWindow : public TScrnSavWindow
{
        DOTS dtArray[NUMDOTS];
        int  nCXScreen, nCYScreen, nDrawIndex, nEraseIndex;
    public:
        TMyScrnSavWindow( PTWindowsObject AParent, LPSTR ATitle,
                          PTModule AModule = NULL );
        virtual LPSTR GetClassName(){   return( szAppName ); }
        virtual void  GetWindowClass( WNDCLASS & AWndClass );
        virtual void  AnimateScreen();
};


TMyScrnSavWindow::TMyScrnSavWindow( PTWindowsObject AParent,
                                    LPSTR ATitle,
                                    PTModule AModule ) :
TScrnSavWindow( AParent, ATitle, AModule )
{
    nDrawIndex   = 0;
    nEraseIndex  = -2*( NUMDOTS/3 );
    nCXScreen    = GetSystemMetrics( SM_CXSCREEN );
    nCYScreen    = GetSystemMetrics( SM_CYSCREEN );
}


void TMyScrnSavWindow::GetWindowClass( WNDCLASS & AWndClass )
{
    TScrnSavWindow::GetWindowClass( AWndClass );
    AWndClass.hbrBackground = ( HBRUSH )GetStockObject( BLACK_BRUSH );
}


void TMyScrnSavWindow::AnimateScreen()
{
    HDC    hDC;

    hDC = GetDC( HWindow );
    if ( hDC )
    {
        SetROP2( hDC, R2_XORPEN );

        dtArray[nDrawIndex].ptVar.x = random( nCXScreen + 1 );
        dtArray[nDrawIndex].ptVar.y = random( nCYScreen + 1 );
        dtArray[nDrawIndex].dwColor = RGB( random( 0x100 ),
                                           random( 0x100 ),
                                           random( 0x100 ));

        SetPixel( hDC, dtArray[nDrawIndex].ptVar.x,
                       dtArray[nDrawIndex].ptVar.y,
                       ( COLORREF )dtArray[nDrawIndex].dwColor );
        nDrawIndex++;
        nDrawIndex %= NUMDOTS;


        if ( nEraseIndex >= 0 )
        {
            SetPixel( hDC, dtArray[nEraseIndex].ptVar.x,
                      dtArray[nEraseIndex].ptVar.y,
                      ( COLORREF )dtArray[nEraseIndex].dwColor );
            nEraseIndex++;
            nEraseIndex %= NUMDOTS;
        }
        else
            nEraseIndex++;

        ReleaseDC( HWindow, hDC );
    }
}


_CLASSDEF( TMyScrnSavDlg )
class TScrnSavDlg : public TDialog
{
    public:
        TScrnSavDlg( PTWindowsObject AParent,
                     LPSTR AName,
                     PTModule AModule = NULL ) :
        TDialog( AParent, AName, AModule )
        {}
};



_CLASSDEF( TMyScrnSavApp )
class TMyScrnSavApp : public TScrnSavApp
{
    public:
        TMyScrnSavApp( LPSTR AName, HINSTANCE AnInstance,
                       HINSTANCE APrevInstance,
                       LPSTR ACmdLine, int ACmdShow ) :
        TScrnSavApp( AName, AnInstance, APrevInstance, ACmdLine, ACmdShow )
        {}
        virtual void  InitScrnSavWindow();
        virtual void  InitConfigDialog();
};


void TMyScrnSavApp::InitScrnSavWindow()
{
    pScrnSavWnd = new TMyScrnSavWindow( NULL, szAppName );
}


void TMyScrnSavApp::InitConfigDialog()
{
    pConfigureDialog = new TScrnSavDlg( NULL, "CONFIGUREDIALOG" );
}


int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine, int nCmdShow )
{
    TMyScrnSavApp App( szAppName, hInstance,
                       hPrevInstance, lpCmdLine, nCmdShow );
    App.Run();
    return( App.Status );
}
