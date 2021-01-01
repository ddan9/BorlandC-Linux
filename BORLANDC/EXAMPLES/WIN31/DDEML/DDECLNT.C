/* Borland C++ - (C) Copyright 1992 by Borland International               */

/***************************************************************************

      Program Name      DDEClnt.c

      Purpose           A simple DDE client application, which communicates
                        to a DDE server using the new 3.1 api DDEML calls.

                        To use this program, build DDEClnt and DDESrvr. There
               			are project files for this.  From the DDEClnt, connect
                        to the server, and send pre-formatted messages to the
                        the server, and request messages from the server.
                        Disconnect before closing app.

****************************************************************************/

#define STRICT

#include <windows.h>
#include <ddeml.h>
#include <dde.h>

#include <stdio.h>
#include <string.h>

#include "ddeclnt.h"


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
HCONV          hConv = NULL;           /*Handle of established conversation*/
HDDEDATA       hData;
DWORD          dwResult;
WORD           wFmt = CF_TEXT;         /*  Clipboard format                */
char           szDDEString[80];        /*  Local allocation of data buffer */
char           szDDEData[80];          /*  Local receive data buffer       */
int            iClientCount = 0;       /*  Client to Server message counter*/
char           tbuf[5];                /*  Temporary, to hold count        */

char szAppName[] = "DDEClientApplication";


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
   wc.hIcon         = LoadIcon ( hInstance, "DDEClientIcon" );
   wc.hCursor       = LoadCursor ( NULL, IDC_ARROW );
   wc.hbrBackground = GetStockObject ( WHITE_BRUSH );
   wc.lpszMenuName  = "DDEClientMenu";
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
                         "DDE Client Window",
                         WS_OVERLAPPEDWINDOW,
                         10,                    /* These co-ordinates look */
                         10,                    /* good on a VGA monitor   */
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
                              APPCMD_CLIENTONLY, 0L ) )
         {
            HandleOutput ( "Client DDE initialization failure." );
            return ( FALSE );
         }

         hszService = DdeCreateStringHandle ( idInst, "Borland", CP_WINANSI );
         hszTopic = DdeCreateStringHandle ( idInst, "DDEExample", CP_WINANSI );
         hszItem = DdeCreateStringHandle ( idInst, "DDEData", CP_WINANSI );

         cCurrentLine = 0;
         cTotalLines = 0;

         strcpy ( szDDEString, "Client application message number:  " );
         break;

      case WM_COMMAND:
         switch ( wParam )
         {
            case IDM_EXIT:
               DestroyWindow ( hWnd );
               break;

            case IDM_CONNECT_SERVER:
               if ( hConv == NULL )
               {
                  hConv = DdeConnect ( idInst, hszService, hszTopic,
                                       (PCONVCONTEXT) NULL );
                  if ( hConv == NULL )
                  {
                     HandleError ( DdeGetLastError ( idInst ) );
                     HandleOutput ( "Unsuccessful connection." );
                  }
                  else
                     HandleOutput ( "Successful connection." );
               }
               else
                  HandleOutput ( "Already connected to DDE Server." );

               break;

            case IDM_DISCONNECT_SERVER:
               if ( hConv != NULL )
               {
                  DdeDisconnect ( hConv );
                  hConv = NULL;
                  HandleOutput ( "Disconnected from server." );
               }
               else
                  HandleOutput ( "Must be connected before disconnecting." );

               break;

            case IDM_MSG_TO_SERVER:
               if ( hConv != NULL )
               {
                  iClientCount ++;
                  sprintf ( tbuf, "%3d.", iClientCount );
                  strncpy ( &szDDEString[36], tbuf, 5 );

                  hData = DdeCreateDataHandle ( idInst, &szDDEString,
                           sizeof ( szDDEString ), 0L, hszItem, wFmt, 0 );

                  if ( hData != NULL )
            		  hData = DdeClientTransaction ( (LPBYTE)hData, -1, hConv,
                               hszItem, wFmt, XTYP_POKE, 1000, &dwResult );
                  else
                     HandleOutput ( "Could not create data handle." );
               }
               else
                  HandleOutput ( "A connection to a DDE Server has not been established." );

               break;

            case IDM_MSG_FROM_SERVER:
               if ( hConv != NULL )
               {
                  hData = DdeClientTransaction ( NULL, 0, hConv,
                               hszItem, wFmt, XTYP_REQUEST, 1000, &dwResult );

                  if ( dwResult == DDE_FNOTPROCESSED )
                     HandleOutput ( "Data not available from server." );
                  else
                  {
                     DdeGetData ( hData, (LPBYTE) szDDEData, 80L, 0L );

                     if ( szDDEData != NULL )
                        HandleOutput ( szDDEData );
                     else
                        HandleOutput ( "Message from server is null." );
                  }
               }
               else
                  HandleOutput ( "A connection to a DDE Server has not been established." );

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
               j = i;         // can't do this if we clear window and start in middle of array

            TextOut ( hDC, 0, y, (LPSTR)(szScreenText[j]),
                                 lstrlen ( szScreenText[j] ) );
            y = y + cyChar;
         }

         EndPaint ( hWnd, &ps );
         break;

      case WM_DESTROY:
         if ( hConv != NULL )
         {
            DdeDisconnect ( hConv );
            hConv = NULL;
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
HDDEDATA EXPENTRY DDECallback ( WORD wType, WORD wFmt, HCONV hConvX, HSZ hsz1,
                                HSZ hsz2, HDDEDATA hData, DWORD dwData1,
                                DWORD dwData2 )
{
   switch ( wType )
   {
      case XTYP_DISCONNECT:
         hConv = NULL;
         HandleOutput ( "The server forced a disconnect." );
         return ( (HDDEDATA) NULL );

      case XTYP_ERROR:
         break;

      case XTYP_XACT_COMPLETE:
         // compare transaction identifier, indicate transaction complete
         break;

   }

   return ( (HDDEDATA) NULL );
}

/***************************************************************************/

void HandleError ( WORD DdeError )
{
   switch ( DdeError )
   {
      case DMLERR_DLL_NOT_INITIALIZED:
         HandleOutput ( "DLL not initialized." );
         break;

      case DMLERR_INVALIDPARAMETER:
         HandleOutput ( "Invalid parameter." );
         break;

      case DMLERR_NO_CONV_ESTABLISHED:
         HandleOutput ( "No conversation established." );
         break;

      case DMLERR_NO_ERROR:
         HandleOutput ( "No error." );
         break;
   }
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

