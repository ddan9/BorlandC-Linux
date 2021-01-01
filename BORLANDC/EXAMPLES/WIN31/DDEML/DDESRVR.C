/* Borland C++ - (C) Copyright 1992 by Borland International               */

/***************************************************************************

      Program Name      DDESrvr.c

      Purpose           A simple DDE client application, which communicates
                        to a DDE server using the new 3.1 api DDEML calls.

                        To use this program, build DDEClnt and DDESrvr. There
                        are project files for this.

****************************************************************************/

#define STRICT

#include <windows.h>
#include <ddeml.h>
#include <dde.h>

#include <stdio.h>
#include <string.h>

#include "ddesrvr.h"


HANDLE         hInst;                  /*  Current instance of application */
HWND           hWnd;                   /*  Handle of Main window           */

short          xScreen;                /* Screen metrics                   */
short          yScreen;                /*  ...                             */
short          yFullScreen;            /*  ...                             */
short          xFrame;                 /*  ...                             */
short          yMenu;                  /*  ...                             */
TEXTMETRIC     tm;                     /* Text metrics                     */
short          cxChar;                 /* Character metrics                */
short          cyChar;                 /*  ...                             */

char           szScreenText[10][80];   /* Contains 10 lines of display data*/
short          cCurrentLine;           /* Index into szScreenText          */
short          cTotalLines;            /* Total lines in szScreenText      */

/*
         The DDE variables
*/

DWORD          idInst = 0L;            /*  Instance of app for DDEML       */
FARPROC        lpDdeProc;              /*  DDE callback function           */
HSZ            hszService;
HSZ            hszTopic;
HSZ            hszItem;
HCONV          hConvApp = NULL;        /*Handle of established conversation*/
char           szDDEData[80];          /*  Local receive buffer            */
char           szDDEString[80];        /*  Local send buffer               */
int            iServerCount = 0;       /*  Send message counter            */
char           tbuf[5];                /*  Temporary buffer for count      */

char szAppName[] = "DDEServerApplication";


/***************************************************************************/

#pragma argsused
int PASCAL WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpszCmdLine, int nCmdShow )
{
	MSG         msg;


   if ( !hPrevInstance )               /* Other instances of app running?  */
      if ( !InitApplication ( hInstance ) ) /* Initialize shared things    */
         return ( FALSE );             /* Exits if unable to initialize    */

   if ( !InitInstance ( hInstance, nCmdShow ) )
      return ( FALSE );

	while ( GetMessage ( &msg, NULL, NULL, NULL ) )
	{
		TranslateMessage ( &msg );
		DispatchMessage ( &msg );
	}

   DdeUninitialize ( idInst );

	return ( msg.wParam );
}

/***************************************************************************/

BOOL FAR PASCAL InitApplication ( HANDLE hInstance )
{
	WNDCLASS    wc;

   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc   = MainWndProc;
   wc.cbClsExtra    = 0;
   wc.cbWndExtra    = 0;
   wc.hInstance     = hInstance;
   wc.hIcon         = LoadIcon ( hInstance, "DDEServerIcon" );
   wc.hCursor       = LoadCursor ( NULL, IDC_ARROW );
   wc.hbrBackground = GetStockObject ( WHITE_BRUSH );
   wc.lpszMenuName  = "DDEServerMenu";
   wc.lpszClassName = szAppName;

   if ( !RegisterClass ( &wc ) )
      return ( FALSE );

   return ( TRUE );
}

/***************************************************************************/

BOOL InitInstance ( HANDLE hInstance, short int nCmdShow )
{
   hInst = hInstance;

   xScreen     = GetSystemMetrics ( SM_CXSCREEN );
   yScreen     = GetSystemMetrics ( SM_CYSCREEN );

	hWnd = CreateWindow ( szAppName,
                         "DDE Server Window",
                         WS_OVERLAPPEDWINDOW,
                         190,                   /* These co-ordinates look */
                         yScreen / 2 - 20,      /* good on a VGA monitor   */
                         xScreen - 200,         /* running in 640x480.  No */
                         yScreen / 2 - 50,      /* combination was tried.  */
                         NULL,
                         NULL,
                         hInstance,
                         NULL );

/*
      If window could not be created, return "failure"
*/

   if ( !hWnd )
      return ( FALSE );

/*
      Make the window visible; update its client area; and return "success"
*/

   ShowWindow ( hWnd, nCmdShow );      /* Show the window                  */
   UpdateWindow ( hWnd );              /* Sends WM_PAINT message           */
   return ( TRUE );              /* Returns the value from PostQuitMessage */

}

/***************************************************************************/

LRESULT CALLBACK MainWndProc ( HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam )
{
   HDC            hDC;
   PAINTSTRUCT    ps;
   DLGPROC        dlgProcAbout;
   int            i;
   int            j;
   short          y;


   switch ( message )
   {
      case WM_CREATE:
         hDC = GetDC ( hWnd );

         GetTextMetrics ( hDC, &tm );
         cxChar = tm.tmAveCharWidth;
         cyChar = tm.tmHeight + tm.tmExternalLeading;

         ReleaseDC ( hWnd, hDC );

         lpDdeProc = MakeProcInstance ( (FARPROC) DDECallback, hInst );
         if ( DdeInitialize ( (LPDWORD)&idInst, (PFNCALLBACK)lpDdeProc,
                              APPCLASS_STANDARD, 0L ) )
         {
            HandleOutput ( "DDE initialization failure." );
            return ( FALSE );
         }
         else
         {
            hszService = DdeCreateStringHandle ( idInst, "Borland", CP_WINANSI );
            hszTopic = DdeCreateStringHandle ( idInst, "DDEExample", CP_WINANSI );
            hszItem = DdeCreateStringHandle ( idInst, "DDEData", CP_WINANSI );

            DdeNameService ( idInst, hszService, (HSZ) NULL, DNS_REGISTER );
         }

         cCurrentLine = 0;
         cTotalLines = 0;

         strcpy ( szDDEString, "Server application message number:  " );
         break;

      case WM_COMMAND:
         switch ( wParam )
         {
            case IDM_EXIT:
               DestroyWindow ( hWnd );
               break;

            case IDM_SHOW_CONNECTIONS:
               if ( hConvApp != NULL )
               {
                  HandleOutput ( "Connection established." );
               }
               else
               {
                  HandleOutput ( "No connection established." );
               }

               break;

            case IDM_ABOUT:
               dlgProcAbout = (DLGPROC) MakeProcInstance ( (FARPROC)About, hInst );
               DialogBox ( hInst, "AboutBox", hWnd, dlgProcAbout );
               FreeProcInstance ( (FARPROC) dlgProcAbout );
               break;

            default:
               return ( DefWindowProc ( hWnd, message, wParam, lParam ) );
         }
         break;

      case WM_PAINT:
         hDC = BeginPaint ( hWnd, &ps );

         y = 0;

         for ( i = 0; i < cTotalLines; i ++ )
         {
            if ( cTotalLines == 8 )
               j = ( (cCurrentLine + 1 + i) % 9 );
            else
               j = i;

            TextOut ( hDC, 0, y, (LPSTR)(szScreenText[j]),
                                 lstrlen ( szScreenText[j] ) );
            y = y + cyChar;
         }

         EndPaint ( hWnd, &ps );
         break;

      case WM_DESTROY:
         if ( hConvApp != NULL )
         {
            DdeDisconnect ( hConvApp );
            hConvApp = NULL;
         }

         DdeFreeStringHandle ( idInst, hszService );
         DdeFreeStringHandle ( idInst, hszTopic );
         DdeFreeStringHandle ( idInst, hszItem );

         FreeProcInstance ( lpDdeProc );

         PostQuitMessage ( 0 );
         break;

      default:
         return ( DefWindowProc ( hWnd, message, wParam, lParam ) );
   }

   return ( FALSE );
}

/***************************************************************************/

#pragma argsused
BOOL CALLBACK About ( HWND hDlg, UINT message,
                        WPARAM wParam, LPARAM lParam )
{
   switch  ( message )
   {
      case WM_INITDIALOG:		   /* message: initialize dialog box */
         return ( TRUE );

      case WM_COMMAND:		      /* message: received a command */
         if ( wParam == IDOK || wParam == IDCANCEL )
         {
            EndDialog ( hDlg, TRUE );	      /* Exits the dialog box	     */
            return ( TRUE );
         }
         break;
   }
   return ( FALSE );			      /* Didn't process a message    */
}

/***************************************************************************/

#pragma argsused
HDDEDATA EXPENTRY DDECallback ( WORD wType, WORD wFmt, HCONV hConv, HSZ hsz1,
                                HSZ hsz2, HDDEDATA hData, DWORD dwData1,
                                DWORD dwData2 )
{
   switch ( wType )
   {
      case XTYP_CONNECT:
         if ( hsz2 == hszService )
         {
            return ( (HDDEDATA) TRUE );
         }
         else
         {
            HandleOutput ( "XTYP_CONNECT: hsz2 != hszService" );
            return ( (HDDEDATA) FALSE );
         }

      case XTYP_REQUEST:
         iServerCount ++;
         sprintf ( tbuf, "%3d.", iServerCount );
         strncpy ( &szDDEString[36], tbuf, 5 );

         hData = DdeCreateDataHandle ( idInst, &szDDEString,
                  sizeof ( szDDEString ), 0L, hszItem, wFmt, 0 );

         if ( hData != NULL )
         {
            HandleOutput ( szDDEString );
            return ( hData );
         }
         else
         {
            HandleOutput ( "Could not create data handle." );
            return ( NULL );
         }

      case XTYP_EXECUTE:
         break;

      case XTYP_POKE:
         if ( hsz1 == hszTopic )
         {
            DdeGetData ( hData, (LPBYTE) szDDEData, 80L, 0L );

            if ( szDDEData != NULL )
            {
               HandleOutput ( szDDEData );
               return ( (HDDEDATA) DDE_FACK );
            }
         }
         else
            return ( (HDDEDATA) NULL );
         break;

      case XTYP_CONNECT_CONFIRM:
         HandleOutput ( "DDE connection confirmed." );
         hConvApp = hConv;
         break;

      case XTYP_DISCONNECT:
         hConvApp = NULL;
         HandleOutput ( "The client has disconnected." );
         break;

      case XTYP_ERROR:
         break;
   }
   
   return ( (HDDEDATA) NULL );
}

/***************************************************************************/

void HandleOutput ( char *szOutputString )
{
   strcpy ( szScreenText[cCurrentLine], szOutputString );
   cCurrentLine = ( cCurrentLine + 1 ) % 9;
   if ( cTotalLines < 8 )
      cTotalLines++;

   InvalidateRect ( hWnd, NULL, TRUE );
   UpdateWindow ( hWnd );
}
