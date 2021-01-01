// ObjectWindows - (C) Copyright 1992 by Borland International
//
// ddesvr.cpp

/*
 * This is a sample application using the OWL library that demonstrats the
 * use of the Windows 3.1 DDEML API in a server application.  You should
 * first build this application and run it.  Run the client application,
 * DDECLI.EXE, to start a conversation with this Server.  Detailed
 * information on DDEML can found in the online help and is suggested
 * reading for anyone interested in writing DDEML applications.  Search on
 * the keyword DDEML.
*/

#include "ddesvr.h"

PTDMLSrWnd pStaticThis = NULL;

TDMLSrApp::TDMLSrApp( LPSTR lpszNam, HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLn, int nCmdShw )
    : TApplication( lpszNam, hInst, hPrevInst, lpszCmdLn, nCmdShw )
{
    nCmdShow = SW_SHOWMINIMIZED;
}

TDMLSrApp::~TDMLSrApp()
{
}

void TDMLSrApp::InitMainWindow()
{
    MainWindow = new TDMLSrWnd( NULL, "DDESVR (A DDE Server)" );
}

/*
There are 16 available timers in Windows.  This little trick uses the
OWL IdleAction() member function to save that scarce resource.
*/
void TDMLSrApp::IdleAction()
{
    static DWORD dwTime = GetTickCount();

    if( (MainWindow != NULL) && (((PTDMLSrWnd)MainWindow)->tfLoop == TRUE) )
    {
        if( (GetTickCount() - dwTime) > 1000 )
        {
            dwTime = GetTickCount(); ((PTDMLSrWnd)MainWindow)->UpdateData();
        }
    }
}

TDMLSrWnd::TDMLSrWnd( PTWindowsObject AParent, LPSTR ATitle )
    : TWindow( AParent, ATitle )
{
}

TDMLSrWnd::~TDMLSrWnd()
{
/*
 * This clean up is required for those resources that were allocated during the
 * DDE conversation.
*/
    if( hConv != 0 )
    {
        DdeDisconnect( hConv );     // Let the other party know we are leaving
        hConv = 0;
    }
    if( idInst != 0 )
    {
        DdeNameService( idInst, hszService, NULL, DNS_UNREGISTER );
        if( hszService != NULL )
        {
            DdeFreeStringHandle( idInst, hszService );
            hszService = NULL;
        }
        if( hszTopic != NULL )
        {
            DdeFreeStringHandle( idInst, hszTopic );
            hszTopic = NULL;
        }
        if( hszItem != NULL )
        {
            DdeFreeStringHandle( idInst, hszItem );
            hszItem = NULL;
        }
        DdeUninitialize(idInst);
        idInst = 0;
    }
    if( lpfnCallBack != NULL )
    {
        FreeProcInstance( lpfnCallBack );
        lpfnCallBack = NULL;
    }
}

void TDMLSrWnd::SetupWindow()
{
    HMENU hMenu;

    pStaticThis = this;
    idInst = 0;
    hConv = 0;
    tfLoop = FALSE;
    hszService = hszTopic = hszItem = 0;
/*
 * The code below sets up the DDE call back function that is used by the
 * DDE Management Library to carry out data transfers between applications.
*/
    lpfnCallBack = MakeProcInstance( (FARPROC)TDMLSrWnd::CallBack, GetApplication()->hInstance );
    if( lpfnCallBack != NULL )
    {
        if( DdeInitialize( &idInst, (PFNCALLBACK)lpfnCallBack, 0, 0L ) == DMLERR_NO_ERROR )
        {
/*
 * The strings below are the 'Service', 'Topic' and 'Item' identifiers that
 * this application makes available, through DDE, to other applications.
*/
            hszService = DdeCreateStringHandle( idInst, "TDMLSR_Server", CP_WINANSI );
            hszTopic = DdeCreateStringHandle( idInst, "Borland", CP_WINANSI );
            hszItem = DdeCreateStringHandle( idInst, "Products", CP_WINANSI );
            if( (hszService != NULL) && (hszTopic != NULL) && (hszItem != NULL) )
            {
                if( DdeNameService( idInst, hszService, NULL, DNS_REGISTER ) != 0 )
                {
// If everything is successful then an About choice is added to the system menu.
                    hMenu = GetSystemMenu( HWindow, 0 );
                    AppendMenu( hMenu, MF_BYCOMMAND | MF_SEPARATOR, -1, "" );
                    AppendMenu( hMenu, MF_BYCOMMAND | MF_STRING, CM_U_ABOUT, "&About DDESVR" );
                } else {
                    MessageBox( HWindow, "Registration failed.", Title, MB_ICONSTOP );
                    PostQuitMessage( 0 );
                }
            } else {
                MessageBox( HWindow, "String creation failed.", Title, MB_ICONSTOP );
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

/*
The code below is used to trap the About menu choice when it is selected
from the system menu.
*/
void TDMLSrWnd::WMSysCommand( RTMessage Msg )
{
    if( (Msg.WParam & 0xFFF0) == CM_U_ABOUT )
    {
        MessageBox( HWindow, "DDESVR.EXE\nWritten using ObjectWindows\nCopyright (c) 1992 by Borland International", "About DDESVR", MB_ICONINFORMATION );
    } else {
        DefWndProc( Msg );
    }
}

/*
This seemingly insignificant function is what keeps this program
minimized, no matter what the user might try to do.
*/
void TDMLSrWnd::WMQueryOpen( RTMessage Msg )
{
    Msg.Result = 0;
}

/*
This function is used to compare incoming Topic and Service requests.
This example DDE Server only makes one Service and one Topic available
so the logic is simple for this case but could be more complex for
Servers that offer multiple Services or Topics.
*/
BOOL TDMLSrWnd::MatchTopicAndService( HSZ hsz1, HSZ hsz2 )
{
    if( DdeCmpStringHandles( hszTopic, hsz1 ) == 0 )
    {
        if( DdeCmpStringHandles( hszService, hsz2 ) == 0 )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*
This function is used to compare incoming Topic and Item pair requests.
This Server only makes one Topic with one Item available so the logic is
simple for this case but could be more complex if the Server offered
multiple Items for multiple Topics.
*/
BOOL TDMLSrWnd::MatchTopicAndItem( HSZ hsz1, HSZ hsz2 )
{
    if( DdeCmpStringHandles( hszTopic, hsz1 ) == 0 )
    {
        if( DdeCmpStringHandles( hszItem, hsz2 ) == 0 )
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*
This function responds to 'system wide' polling of any available
Services with specific Topics, any Topics with specific Services or any
Services with any Topics.  It simply replies with a match, if there is
one, so that the polling application can open data discussions later, if
desired.
*/
HDDEDATA TDMLSrWnd::WildConnect( HSZ hsz1, HSZ hsz2, WORD wFmt )
{
    HSZPAIR hszpTemp[] = { { hszService, hszTopic }, { 0, 0 } };

    if( (hsz1 == NULL) && (hsz2 == NULL) )      // Returns all if true
    {
        return DdeCreateDataHandle( idInst, (LPBYTE)&hszpTemp[0], sizeof( hszpTemp ), 0L, 0, wFmt, 0 );
    }
    if( (hsz1 == NULL) && (DdeCmpStringHandles( hsz2, hszService ) == 0) )
    {
        return DdeCreateDataHandle( idInst, (LPBYTE)&hszpTemp[0], sizeof( hszpTemp ), 0L, 0, wFmt, 0 );
    }
    if( (DdeCmpStringHandles( hsz1, hszTopic) == 0) && (hsz2 == NULL) )
    {
        return DdeCreateDataHandle( idInst, (LPBYTE)&hszpTemp[0], sizeof( hszpTemp ), 0L, 0, wFmt, 0 );
    }
    return NULL;
}

HDDEDATA TDMLSrWnd::DataRequested( WORD wFmt )
{
    static int iLoop = 0;       // Loop counter for the array below
    static char szItems[][42] =
    {
        "Borland C++",
        "Turbo C++",
        "Turbo Pascal",
        "ObjectVision",
        "Paradox",
        "dBase",
        "Quattro Pro",
        "Brief",
        "Borland Version Control"
    };

    if( wFmt == CF_TEXT )
    {
        iLoop++;
        iLoop %= (sizeof( szItems ) / sizeof( szItems[0] ));
        return DdeCreateDataHandle( idInst, &szItems[iLoop], sizeof( szItems[iLoop] ), 0, hszItem, wFmt, 0 );
    }
    return NULL;
}

/*
This is triggered by the IdleAction() loop above whenever the user
enters an advise loop.
*/
void TDMLSrWnd::UpdateData( void )
{
    DdePostAdvise( idInst, hszTopic, hszItem );
}

/*
This call back function is the heart of interaction between this program
and DDE.  Because Windows doesn't pass C++ 'this' pointers to call
back functions, a static 'this' pointer was used.  If you wanted to
create a Server that would allow for more than one conversation, using a
List of conversations and their associated 'this' pointers would be one
possible method to try.  The XTYP_ constants are described in detail in
the online help.
*/

HDDEDATA FAR PASCAL TDMLSrWnd::CallBack( WORD wType, WORD wFmt, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD, DWORD )
{
    char szTemp[128];
    int iSize;

    switch( wType )
    {
        case XTYP_ADVREQ :
            if( pStaticThis->MatchTopicAndItem( hsz1, hsz2 ) == TRUE )
            {
                return pStaticThis->DataRequested( wFmt );
            }
            return NULL;
        case XTYP_ADVSTART :
            if( (pStaticThis->tfLoop == FALSE) && (pStaticThis->MatchTopicAndItem( hsz1, hsz2 ) == TRUE) )
            {
                pStaticThis->tfLoop = TRUE;
                return (HDDEDATA)1;
            } else {
                return 0;
            }
        case XTYP_ADVSTOP :
            if( (pStaticThis->tfLoop == TRUE) && (pStaticThis->MatchTopicAndItem( hsz1, hsz2 ) == TRUE) )
            {
                pStaticThis->tfLoop = FALSE;
            }
            break;
        case XTYP_CONNECT :
            if( pStaticThis->hConv == 0 )
            {
                if( pStaticThis->MatchTopicAndService( hsz1, hsz2 ) == TRUE )
                {
                    return (HDDEDATA)1;
                }
            }
            return 0;
        case XTYP_CONNECT_CONFIRM :
            pStaticThis->hConv = hConv;
            break;
        case XTYP_DISCONNECT :
            if( hConv == pStaticThis->hConv )
            {
                pStaticThis->hConv = 0;
                pStaticThis->tfLoop = FALSE;
            }
            break;
        case XTYP_ERROR :
            MessageBox( pStaticThis->HWindow, "A critical DDE error has occured.", pStaticThis->Title, MB_ICONINFORMATION );
            break;
        case XTYP_EXECUTE :
            return DDE_FNOTPROCESSED;
        case XTYP_POKE :
            wsprintf( szTemp, "The server received : " );
            iSize = strlen( szTemp );
            DdeGetData( hData, &szTemp[iSize], (sizeof( szTemp ) - iSize), 0 );
            MessageBox( GetFocus(), szTemp, pStaticThis->Title, MB_ICONINFORMATION );
            return (HDDEDATA)DDE_FACK;
        case XTYP_REQUEST :
            if( pStaticThis->MatchTopicAndItem( hsz1, hsz2 ) == TRUE )
            {
                return pStaticThis->DataRequested( wFmt );
            }
            return NULL;
        case XTYP_WILDCONNECT :
            return pStaticThis->WildConnect( hsz1, hsz2, wFmt );
        default :
            break;
    }
    return NULL;
}

int PASCAL WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLn, int nCmdShw )
{
    TDMLSrApp App( "DDESVR Application", hInst, hPrevInst, lpszCmdLn, nCmdShw );

    App.Run();
    return App.Status;
}

