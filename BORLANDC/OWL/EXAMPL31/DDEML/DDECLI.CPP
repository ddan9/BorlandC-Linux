// ObjectWindows - (C) Copyright 1992 by Borland International
//
// ddecli.cpp

/*
 * This is a sample application using the OWL library that demonstrats the
 * use of the Windows 3.1 DDEML API in a client application.  You should
 * first build the server application, DDESVR.EXE, and run it.  Then run the
 * client application, DDECLI.EXE, to start a conversation.  Detailed
 * information on DDEML can found in the online help and is suggested
 * reading for anyone interested in writing DDEML applications.  Search on
 * the keyword DDEML.
*/

#include "ddecli.h"

PTDMLClWnd pStaticThis = NULL;

TDMLClApp::TDMLClApp( LPSTR lpszNam, HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLn, int nCmdShw )
    : TApplication( lpszNam, hInst, hPrevInst, lpszCmdLn, nCmdShw )
{
}

TDMLClApp::~TDMLClApp()
{
}

void TDMLClApp::InitMainWindow()
{
    MainWindow = new TDMLClWnd( NULL, "DDECLI (A DDE Client)" );
}

TDMLClWnd::TDMLClWnd( PTWindowsObject AParent, LPSTR ATitle )
    : TWindow( AParent, ATitle )
{
}

TDMLClWnd::~TDMLClWnd()
{
/*
 * This clean up is required for those resources that were allocated during the
 * DDEML conversation.
*/
    if( hConv != 0 )
    {
        DdeDisconnect( hConv );     // Let the other party know we are leaving
        hConv = 0;
    }
    if( idInst != 0 )
    {
        DdeFreeStringHandle( idInst, hszService );
        DdeFreeStringHandle( idInst, hszTopic );
        DdeFreeStringHandle( idInst, hszItem );
        DdeUninitialize(idInst);
        idInst = 0;
    }
    if( lpCallBack != NULL )
    {
        FreeProcInstance( (FARPROC)lpCallBack );
        lpCallBack = NULL;
    }
}

void TDMLClWnd::SetupWindow()
{
    AssignMenu( TDMLClWnd_MENU );
    pStaticThis = this;
    idInst = 0;         // MUST be 0 the first time DdeInitialize() is called!
    hConv = 0;
    tfLoop = 0;
    szData[0] = '\0';
    hszService = hszTopic = hszItem = 0;
/*
 * The code below sets up the DDEML call back function that is used by the
 * DDE Management Library to carry out data transfers between applications.
*/
    lpCallBack = MakeProcInstance( (FARPROC)TDMLClWnd::CallBack, GetApplication()->hInstance );
    if( lpCallBack != NULL )
    {
        if( DdeInitialize( &idInst, (PFNCALLBACK)lpCallBack, APPCMD_CLIENTONLY, 0 ) == DMLERR_NO_ERROR )
        {
            hszService = DdeCreateStringHandle( idInst, "TDMLSR_Server", CP_WINANSI );
            hszTopic = DdeCreateStringHandle( idInst, "Borland", CP_WINANSI );
            hszItem = DdeCreateStringHandle( idInst, "Products", CP_WINANSI );
            if( (hszService == NULL) || (hszTopic == NULL) || (hszItem == NULL) )
            {
                MessageBox( HWindow, "Creation of strings failed.", Title, MB_ICONSTOP );
                PostQuitMessage( 0 );
            }
        } else {
            MessageBox( HWindow, "Initialization failed.", Title, MB_ICONSTOP );
            PostQuitMessage( 0 );
        }
    } else {
        MessageBox( HWindow, "Setup of callback failed.", Title, MB_ICONSTOP );
        PostQuitMessage( 0 );
    }
}

void TDMLClWnd::Paint( HDC hdc, PAINTSTRUCT _FAR & )
{
    RECT rTemp;
    char szTemp[] = "This example of the Dynamic Data Exchange Management \
Library obtains the names of various Borland products from the server \
DDESVR.EXE. To get started, first run DDESVR.EXE and then select the \"Connect!\" menu item.";

    GetClientRect( HWindow, &rTemp );
    if( szData[0] != '\0' )
    {
// The szData string is obtained from the DDESVR.EXE DDE Server.
        DrawText( hdc, szData, strlen( szData ), &rTemp, DT_SINGLELINE | DT_CENTER | DT_VCENTER );
    } else {
        DrawText( hdc, szTemp, strlen( szTemp ), &rTemp, DT_WORDBREAK );
    }
}

void TDMLClWnd::CMExit( RTMessage Msg )
{
    TWindow::CMExit( Msg );
}

void TDMLClWnd::WMInitMenu( RTMessage Msg )
{
    HMENU hmTemp;

/*
This trick is used to automatically update the status of the various
menu choices just before it is displayed.
*/
    hmTemp = (HMENU)Msg.WParam;
    EnableMenuItem( hmTemp, CM_U_CONNECT, ( hConv == 0 )? MF_ENABLED : MF_GRAYED );
    EnableMenuItem( hmTemp, CM_U_REQUEST, ( hConv != 0 )? MF_ENABLED : MF_GRAYED );
    EnableMenuItem( hmTemp, CM_U_POKE, ( hConv != 0 )? MF_ENABLED : MF_GRAYED );
    EnableMenuItem( hmTemp, CM_U_ADVISE, ( hConv != 0 )? MF_ENABLED : MF_GRAYED );
    CheckMenuItem( hmTemp, CM_U_ADVISE, MF_BYCOMMAND | ( tfLoop == TRUE )? MF_CHECKED : MF_UNCHECKED );
    DrawMenuBar( HWindow );
}

// The following 4 functions are used to communicate with DDE Server(s).

void TDMLClWnd::CMUConnect( RTMessage )
{
    hConv = DdeConnect( idInst, hszService, hszTopic, NULL );
    if( hConv != 0 )
    {
        PostMessage( HWindow, WM_INITMENU, (WPARAM)GetMenu( HWindow ), 0 );
    } else {
        MessageBox( HWindow, "Can't start conversation.\nTry running DDESVR (the server).", Title, MB_ICONSTOP );
    }
}

void TDMLClWnd::CMURequest( RTMessage )
{
    HDDEDATA hData;

    hData = DdeClientTransaction( NULL, 0, hConv, hszItem, CF_TEXT, XTYP_REQUEST, 0, NULL );
    if( hData != FALSE ) ReceivedData( hData );
}

void TDMLClWnd::CMUPoke( RTMessage )
{
    char szTemp[42] = "";

    if( GetApplication()->ExecDialog( new TInputDialog( this, Title, "Poke string : ", szTemp, sizeof( szTemp ) ) ) == IDOK )
    {
        DdeClientTransaction( (LPBYTE)szTemp, strlen( szTemp ) + 1, hConv, hszItem, CF_TEXT, XTYP_POKE, 1000, NULL );
    }
}

void TDMLClWnd::CMUAdvise( RTMessage Msg )
{
    HMENU hTempMenu;
    DWORD dwTempResult;

    hTempMenu = GetMenu( HWindow );
    if( GetMenuState( hTempMenu, Msg.WParam, MF_BYCOMMAND ) == MF_UNCHECKED )
    {
        if( (BOOL)DdeClientTransaction( NULL, 0, hConv, hszItem, CF_TEXT, XTYP_ADVSTART | XTYPF_ACKREQ, 1000, &dwTempResult ) == TRUE )
        {
            CheckMenuItem( hTempMenu, Msg.WParam, MF_BYCOMMAND | MF_CHECKED );
            tfLoop = TRUE;
        }
    } else {
        if( (BOOL)DdeClientTransaction( NULL, 0, hConv, hszItem, CF_TEXT, XTYP_ADVSTOP, 1000, &dwTempResult ) == TRUE )
        {
            CheckMenuItem( hTempMenu, Msg.WParam, MF_BYCOMMAND | MF_UNCHECKED );
            tfLoop = FALSE;
        }
    }
    DrawMenuBar( HWindow );
}

void TDMLClWnd::CMUHelpAbout( RTMessage )
{
    MessageBox( HWindow, "DDECLI.EXE\nWritten using ObjectWindows\nCopyright (c) 1992 by Borland International", "About DDECLI", MB_ICONINFORMATION );
}

void TDMLClWnd::ReceivedData( HDDEDATA hData )
{
/*
This function is called when the callback function is notified of
available data.
*/
    if( hData != NULL )
    {
        DdeGetData( hData, &szData, sizeof( szData ), 0 );
        InvalidateRect( HWindow, NULL, TRUE );
    }
}

/*
This call back function is the heart of interaction between this program
and DDEML.  Because Windows doesn't pass C++ 'this' pointers to call
back functions, a static 'this' pointer was used.  If you wanted to
create a Client that would allow for more than one conversation, using a
List of conversations and their associated 'this' pointers would be one
possible method to try.  The XTYP_ constants are described in detail in
the online help.
*/
HDDEDATA FAR PASCAL _export TDMLClWnd::CallBack( WORD wType, WORD, HCONV hConv, HSZ, HSZ, HDDEDATA hData, DWORD, DWORD )
{
    switch( wType )
    {
        case XTYP_ADVDATA :
            if( hConv == pStaticThis->hConv )
            {
                pStaticThis->ReceivedData( hData );
            }
            return (HDDEDATA)DDE_FACK;
        case XTYP_XACT_COMPLETE :
            if( hConv == pStaticThis->hConv ) pStaticThis->ReceivedData( hData );
            break;
        case XTYP_DISCONNECT :
            MessageBox( pStaticThis->HWindow, "Disconnected.", pStaticThis->Title, MB_ICONINFORMATION );
            pStaticThis->hConv = 0;
            pStaticThis->tfLoop = 0;
            PostMessage( pStaticThis->HWindow, WM_INITMENU, (WPARAM)GetMenu( pStaticThis->HWindow ), 0 );
            break;
        case XTYP_ERROR :
            MessageBox( pStaticThis->HWindow, "A critical DDE error has occured.", pStaticThis->Title, MB_ICONINFORMATION );
            break;
        default :
            break;
    }
    return NULL;
}

int PASCAL WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLn, int nCmdShw )
{
    TDMLClApp App( "DDECLI Application", hInst, hPrevInst, lpszCmdLn, nCmdShw );

    App.Run();
    return App.Status;
}

