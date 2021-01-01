// Windows 3.1 Demo Program
// Copyright (c) 1992 by Borland International

// File CLIOLE.C
// An example of an Ole Client.  It demonstrates
// a simple use of Client Ole functions.

// Its main window allows the user to create a paint brush object, or
// copy an object in from the clipboard.  Note, you do need to have
// paint brush available on the system.  (It is a standard part of
// Windows 3.1).

#define STRICT

#include <windows.h>
#include <ole.h>
#include <commdlg.h>
#include <process.h>
#include <alloc.h>
#include <dir.h>
#include <dos.h>
#pragma hdrstop

#include "clioleer.h"
#include "cliole.h"
#include "cliolerc.h"

char szAppName[] = "ClientOleApp" ;

HINSTANCE hInst = 0;

void InitApplication(void)
{
	WNDCLASS wndclass;

	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
	wndclass.lpfnWndProc   = (WNDPROC) WndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = hInst ;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndclass.lpszMenuName  = "CLIOLEMENU" ;
	wndclass.lpszClassName = szAppName ;
	RegisterClass (&wndclass) ;
}

// These are clipboard formats that Ole
// applications use to determine the type of
// object on the clipboard.  This application does not
// make use of them.

OLECLIPFORMAT vcfLink, vcfNative, vcfOwnerLink;

void InitInstance(void)
{
	vcfLink = RegisterClipboardFormat( "ObjectLink" );
	vcfNative = RegisterClipboardFormat( "Native" );
	vcfOwnerLink = RegisterClipboardFormat( "OwnerLink" );
}

#pragma argsused
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpszCommandLine, int nCmdShow)
	 {
	 HWND        hwnd ;
	 MSG         msg ;

     hInst = hInstance;
	 if (!hPrevInstance)
	 {
		InitApplication();
	 }

	 InitInstance();

	 hwnd = CreateWindow (szAppName,
			"Client OLE Program",
					WS_OVERLAPPEDWINDOW,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					NULL,
					NULL,
					hInstance,
					NULL) ;

	 ShowWindow (hwnd, nCmdShow) ;
	 UpdateWindow (hwnd) ;

	 while (GetMessage (&msg, NULL, 0, 0))
	 {
		  TranslateMessage (&msg) ;
		  DispatchMessage (&msg) ;
	 }
	 return msg.wParam ;
}



int FAR PASCAL _export StdCallBack(LPOLECLIENT lpClient,
	OLE_NOTIFICATION notification,
	LPOLEOBJECT lpObject )
{
	static TOleNot OleNotStruct;
	OleNotStruct.lpClient = lpClient;
	OleNotStruct.not      = notification;
	OleNotStruct.lpObject = lpObject;

	return (
		(int)SendMessage( (( TOleClient * )lpClient)->hwnd,
			WM_U_CALLBACK, 0, (DWORD)&OleNotStruct )
			);
}

// It should be noted that since much of the data associated with the
// main window is stored globally, only one Windows class OleAppName
// is supported.  Extending the program to support several windows of this
// class would require that you store this data on a per window basis.
// A good technique would be to use a structure which contains all
// these data members, and to store a pointer to such a structure
// in the extra bytes of the windows.

	OLESTATUS ret;   		//last OleStatus
	LHCLIENTDOC lhClientDoc;
	char lpszDocName[ MAXPATH ];
	BOOL bDefDocName;

	char lpszObjectName[ MAXPATH ];
	char lpszLastObjectName[ MAXPATH ];
	TOleClient *pOleClient;
	BOOL bObjectLoaded = FALSE;
	BOOL bUndoObjectLoaded = FALSE;
	LPOLEOBJECT lpObject = NULL;		// actual object we are housing
	LPOLEOBJECT lpUndoObject = NULL;	// last change made to object
	int nNextObjectNum = 0;


long FAR PASCAL _export WndProc (HWND hwnd, WORD message, WORD wParam, LONG lParam)
{

   HDC           hdc ;
	PAINTSTRUCT   ps ;
	RECT	 	     rect ;
   TOleNot far * ons ;

	switch (message)
	{
		case WM_U_REDRAW:
			WMURedraw( hwnd );
			return 0;

		case WM_COMMAND:
			switch ( wParam )
			{
				case CM_PBRUSH:
					CMPBrush();
					break;

				case CM_UNDO:
					CMUndo(hwnd);
					break;

				case CM_ACTIVATE:
					CMActivate( hwnd );
					break;

				case CM_CLEAR:
					CMClear( hwnd );
					break;

				case CM_CUT:
					CMCut(hwnd);
					break;

				case CM_COPY:
					CMCopy(hwnd);
					break;

				case CM_PASTE:
					CMPaste(hwnd);
					break;

				case CM_ABOUT:
					CMAbout( hwnd );
					break;

				case CM_EXIT:
					DestroyWindow( hwnd );
					break;
			}
			return 0;

		case WM_CREATE:
			WMCreate( hwnd );
			return 0;

		case WM_INITMENU:
			WMInitMenu( hwnd , wParam );
			return 0;

		case WM_U_CALLBACK:
			(TOleNot far *)ons = (TOleNot far *)lParam;
			WMUCallBack( ons->lpClient, ons->not, ons->lpObject );
			return 0;

		case WM_PAINT:
			 hdc = BeginPaint (hwnd, &ps) ;
			 GetClientRect (hwnd, &rect) ;

			 Paint( hwnd , hdc );

			 EndPaint (hwnd, &ps) ;
			 return 0 ;

		case WM_DESTROY:
			PostQuitMessage (0) ;
			return 0 ;
	}
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}


// Comments on MakeTOleClient:
// If no vtbl has been constructed, make one.  Then initialize
// the vtbl member of the struct to point to it.
// To be more robust, one should always check that memory is allocated,
// and MakeProcInstance succeeds.

LPOLECLIENTVTBL lpClientVtbl = NULL;

void MakeTOleClient( TOleClient *OleClient, HWND owner , HINSTANCE hInst )
{
	OleClient->hwnd = owner;
	if ( !lpClientVtbl )
	{
		lpClientVtbl = (LPOLECLIENTVTBL) malloc( sizeof ( OLECLIENTVTBL ));
		lpClientVtbl->CallBack = (TCallBack)MakeProcInstance( (FARPROC)StdCallBack,
					hInst );
	}
	OleClient->oleClient.lpvtbl = lpClientVtbl;
}

void WMURedraw(HWND hwnd )
{
	bObjectLoaded = TRUE;
	InvalidateRect( hwnd, NULL, TRUE );
	UpdateWindow( hwnd );
}

#pragma argsused
int WMUCallBack( LPOLECLIENT lpOleClient ,
				 OLE_NOTIFICATION oleNot,
				 LPOLEOBJECT lpOleObject )
{
	switch ( oleNot ) {
		case OLE_CHANGED:
		case OLE_SAVED:
			PostMessage( (( TOleClient *)lpOleClient)->hwnd ,
				WM_U_REDRAW, 0, 0L );
			break;

		case OLE_CLOSED:
			break;

		case OLE_QUERY_PAINT:
			break;

		case OLE_RELEASE:
			break;

		case OLE_RENAMED:
			break;

		default:
			break;
	}
	return TRUE;
}

// create a new paint brush
void CMPBrush(void)
{
	BackupObject();
	bObjectLoaded = FALSE;

	lstrcpy( lpszObjectName, GetNextObjectName() );

	ret = OleCreate( "StdFileEditing",
			(LPOLECLIENT)pOleClient,
			"PBRUSH",
			lhClientDoc,
			szAppName,
			&lpObject,
			olerender_draw,
			0 );

   // Creating an Ole Object is an asynchronous operation.  An
	// interesting experiment is to use TDW to step into
	// WaitOleNotBusy (which the following wait macro takes you
	// to) and look at the user screen between message
	// dispatching.  You should see pbrush gradually paint
	// itself as it processes the messages which which Ole
   // generates for it.  In general, if an Ole Server does not
	// behave properly when creating an object, a likely cause is a
	// problem with the message dispatch loop.

	wait( ret , lpObject );

	// OleSetHostNames sets the name in the server app.  If this
	// was not called, pbrush would display a string with a bunch
   // of % signs in it.

	ret = OleSetHostNames( lpObject, szAppName, lpszObjectName );
	wait( ret , lpObject );
}

void CMUndo( HWND hwnd )
{
	if ( lpUndoObject )		// we have a previous object
		if ( lpUndoObject != lpObject )	// it is different then the current one
		{
			LPOLEOBJECT lpObjectToDelete = lpObject;
			lpObject = lpUndoObject;
			ret = OleDelete( lpObjectToDelete );
			wait( ret , lpObjectToDelete );
			bObjectLoaded = bUndoObjectLoaded;

			InvalidateRect( hwnd, NULL, TRUE );
         UpdateWindow( hwnd );
		}
}

void CMCut(HWND hwnd )
{
	CMCopy(hwnd);
	CloseCurrentOle(hwnd);
}

void CMCopy( HWND hwnd )
{
	if ( OpenClipboard( hwnd ) && EmptyClipboard() )
	{
		ret = OleCopyToClipboard( lpObject );
		check( ret );
		CloseClipboard();
	}
}

void BackupObject(void)
{
	if ( lpObject )
   {
		ret = OleClone( lpObject, (LPOLECLIENT)pOleClient, lhClientDoc, szAppName, &lpUndoObject );
		wait( ret, lpObject );
		lstrcpy( lpszLastObjectName, lpszObjectName );
		lstrcpy( lpszObjectName , GetNextObjectName() );
		bUndoObjectLoaded = bObjectLoaded;
	}
}


void CMPaste( HWND hwnd )
{
	if ( OpenClipboard( hwnd ) )
	{
		BackupObject();

		ret = OleCreateFromClip( "StdFileEditing",
			(LPOLECLIENT)pOleClient,
			lhClientDoc,
			lpszObjectName,
			&lpObject,
			olerender_draw,
			0 );
		check( ret );

		bObjectLoaded = TRUE;

		CloseClipboard();
		PostMessage( hwnd , WM_U_REDRAW, 0, 0L );
	}
}

LPSTR GetNextObjectName(void)
{
	static char buffer[ MAXPATH ];
	wsprintf( buffer, "object #%03d", nNextObjectNum++ );
	return buffer;
}

void Paint( HWND hwnd, HDC hdc )
{
	LPOLEOBJECT lpObjectToDraw = NULL;

	if ( bObjectLoaded )
		lpObjectToDraw = lpObject;
	else if ( lpUndoObject )
		lpObjectToDraw = lpUndoObject;

	if ( lpObjectToDraw ) {
		RECT rect;
		GetClientRect( hwnd, &rect );

		// Tips for OleDraw:
		// OleDraw will return OLE_ERROR_OBJECT if the
      // object to be drawn is invalid.  A case such as this,
      // could happen if the Paint function does not get
		// informed about the object being deleted, or not having
		// yet been created.
		// You can also get OLE_ERROR_BLANK if you draw an object
		// which has not been stored.  A time when this could happen
      // is when trying use OleDraw on the object after using OleCreate
		// to make it, but not yet using the Servers update command
		// to save the object with Ole.
		ret = OleDraw( lpObjectToDraw , hdc, &rect , NULL, 0 );
		wait( ret, lpObjectToDraw );
	}
}

void WMCreate( HWND hwnd )
{
	ret = OLE_OK;
	lhClientDoc = 0;

	bObjectLoaded = FALSE;
	bUndoObjectLoaded = FALSE;

	pOleClient = NULL;
	lpObject = NULL;
	lpUndoObject = NULL;

	lstrcpy( lpszDocName , "noname.ole" );
	*lpszLastObjectName = 0;
	*lpszObjectName = 0;
	bDefDocName = TRUE;
	RegisterClientDoc();
	pOleClient = (TOleClient *) malloc( sizeof( TOleClient ));
	if ( pOleClient )
		MakeTOleClient( pOleClient, hwnd , hInst );
	else memerror(  );
}


void RegisterClientDoc(void)
{
	// Tip for using OleRegisterClientDoc:
   // If the lpszDoc parameter passed to
	// OleRegisterClientDoc is not valid, (lpszPatnName in this case),
	// one can get an OLE_ERROR_ADDRESS error, and the ClientDoc
	// handle will not be valid.

	ret = OleRegisterClientDoc(
			szAppName,
			lpszDocName,
			0,
			&lhClientDoc );
	check( ret );
}

void RegFileName( LPSTR FileName )
{
	lstrcpy( lpszDocName , FileName );
	ret = OleRegisterClientDoc( szAppName,
			lpszDocName ,
			0,
			&lhClientDoc );
	check ( ret );
}

void CMActivate( HWND hwnd )
{
	RECT rect;
	BackupObject();
	GetClientRect( hwnd, &rect );
	ret = OleActivate( lpObject , OLEVERB_PRIMARY, TRUE, TRUE ,
			hwnd , &rect );
	wait ( ret, lpObject );
	PostMessage( hwnd , WM_U_REDRAW, 0, 0L );
}

void WMInitMenu( HWND hwnd , WORD wParam )
{
	WORD wEnableUndo;
	HMENU hMenu = (HMENU)wParam;

	if ( (lpObject != lpUndoObject) && ( lpUndoObject != NULL ))
		wEnableUndo = MF_ENABLED;
	else wEnableUndo = MF_GRAYED;
	EnableMenuItem( hMenu, CM_UNDO     , wEnableUndo );
	EnableMenuItem( hMenu, CM_COPY     , ( bObjectLoaded ? MF_ENABLED : MF_GRAYED ));
	EnableMenuItem( hMenu, CM_CUT      , ( bObjectLoaded ? MF_ENABLED : MF_GRAYED ));
	ret = OleQueryCreateFromClip( "StdFileEditing", olerender_draw, 0 );
	EnableMenuItem( hMenu, CM_PASTE    , (( ret == OLE_OK ) ? MF_ENABLED : MF_GRAYED ));
	EnableMenuItem( hMenu, CM_ACTIVATE , ( bObjectLoaded ? MF_ENABLED : MF_GRAYED ));
	EnableMenuItem( hMenu, CM_CLEAR    , ( bObjectLoaded ? MF_ENABLED : MF_GRAYED ));

	DrawMenuBar( hwnd );
}

void CMClear( HWND hwnd )
{
	CloseCurrentOle(hwnd);
}

void CMAbout( HWND hwnd )
{
    MessageBox( hwnd , "OLE Client Example\nCopyright (c) 1992 Borland International, Inc.", szAppName, MB_OK );
}

void CloseCurrentOle( HWND hwnd )
{
	bObjectLoaded = FALSE;

	if ( lpObject ) {
		ret = OleDelete( lpObject );
		wait( ret , lpObject );
	}
	if ( lpUndoObject ) {
		ret = OleDelete( lpUndoObject );
		wait( ret , lpObject );
	}
	lpObject = lpUndoObject = NULL;

	InvalidateRect( hwnd , NULL, TRUE );
	UpdateWindow( hwnd );
}

void memerror(void)
{
	MessageBox( GetFocus() , "error allocating memory", szAppName, MB_OK );
	exit(1);
}

// General Help:
// Question: Right after closing down the server, the only part of the
//   object repainted in my application is that portion of the window that
//   was covered with the server program, what's going on?
// Answer: When the server closes, Windows sends you a paint message to
//	 update that portion of your window which the server covered up.  It
//	 has a clipping region defined which keeps all the previous visible
//	 portions of your client window.  To repaint the whole window after
//	 a server closes, respond to the OLE_CHANGED notification in your
//	 callback, and force the whole window to be updated there.  Calling
//	 InvalidateRect would be appropriate.

// _lopen will not create a new file.  Use OpenFile( .. , OF_CREATE ) or
// _lcreate for this.

