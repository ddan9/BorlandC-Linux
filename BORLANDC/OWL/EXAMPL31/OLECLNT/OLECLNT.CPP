// ObjectWindows - (C) Copyright 1992 by Borland International
//
// oleclnt.cpp

// An example of an Ole Client program using OWL.  It demonstrates
// a simple use of Client Ole functions, and using C++ classes to
// wrap around the Ole structures.

// Its main window allows the user to create a paint brush object, or
// copy an object in from the clipboard.  Note, you do need to have
// paint brush available on the system.  (It is a standard part of
// Windows 3.0 or 3.1).

#include <owl.h>
#include <listbox.h>
#include <string.h>
#include <edit.h>
#include <bwcc.h>
#include <filedial.h>
#include <ole.h>
#include <shellapi.h>
#pragma hdrstop

#include "oleclnte.h"
#include "oleclntr.h"
#include "oleclnt.h"

// static class data members.
LPOLECLIENTVTBL TOwlClient::lpClientVtbl = NULL;
int             TOleDocWindow::nNextObjectNum = 0;

void TOleApp::InitInstance()
{
	TApplication::InitInstance();

	vcfLink = RegisterClipboardFormat( "ObjectLink" );
	vcfNative = RegisterClipboardFormat( "Native" );
	vcfOwnerLink = RegisterClipboardFormat( "OwnerLink" );
	// comments in owlole.h mention these ole clipboard formats
}


// passes the notification on to the OWL Object which
// is keeping track of this Ole Object

int FAR PASCAL _export StdCallBack(LPOLECLIENT lpClient,
	OLE_NOTIFICATION notification,
	LPOLEOBJECT lpObject )
{
	return (( PTOwlClient )lpClient)->TOleDocWindowThis->
		CallBack( lpClient ,
		notification,
		lpObject );
}

// if no vtbl has been constructed, make one.  Then initialize
// the vtbl class instance member to point to it.  When smart
// call backs are used for the compiler entry/exit code,
// MakeProcInstance is not needed.  This code will not call
// MakeProcInstance when the default value of hInst is used (it
// defaults to being 0).  If you want to use Procedure instances
// for the callbacks, construct a TOwlClient with a valid hInst.

TOwlClient::TOwlClient( PTOleDocWindow owner , HINSTANCE hInst )
{
	TOleDocWindowThis = owner;
	if ( !lpClientVtbl )
	{
		lpClientVtbl = new OLECLIENTVTBL;
		if (  hInst == 0 ) {
			lpClientVtbl->CallBack = StdCallBack;
		} else {
			lpClientVtbl->CallBack = (TCallBack)MakeProcInstance( (FARPROC)StdCallBack,
					hInst );
		}
	}
	lpvtbl = lpClientVtbl;
}

void TOleDocWindow::WMURedraw( RTMessage )
{
	bObjectLoaded = TRUE;
	InvalidateRect( HWindow, NULL, TRUE );
	UpdateWindow( HWindow );
}

#pragma argsused
int TOleDocWindow::CallBack( LPOLECLIENT      lpOleClient ,
							 OLE_NOTIFICATION oleNot,
							 LPOLEOBJECT lpOleObject )
{
	switch ( oleNot ) {
		case OLE_CHANGED:
		case OLE_SAVED:
			PostMessage( HWindow , WM_U_REDRAW, 0, 0L );
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

void TOleDocWindow::CMAbout( RTMessage )
{

	MessageBox( HWindow , "OLE Client Program\nWritten using ObjectWindows\nCopyright (c) 1992 Borland",
			GetApplication()->Name, MB_OK );
}

// create a new paint brush
void TOleDocWindow::CMPBrush( RTMessage )
{
	BackupObject();
	bObjectLoaded = FALSE;

	lstrcpy( lpszObjectName, GetNextObjectName() );

	ret = OleCreate( "StdFileEditing",
			(LPOLECLIENT)pOwlClient,
			"PBRUSH",
			lhClientDoc,
			GetApplication()->Name,
			&lpObject,
			olerender_draw,
			0 );

	// Creating an Ole Object is a asynchronous operation.  An
	// interesting experiment is to use TDW to step into
	// WaitOleNotBusy (which the following wait macro takes you
	// to) and look at the user screen between message
	// dispatching.  You should see pbrush gradually paint
	// itself as it processes the messages which which Ole
	// generates for it.  In general, if a Ole Server does not
	// behave properly when creating an object, a likely cause is a
	// problem with the message dispatch loop.

	wait( ret , lpObject );

	// OleSetHostNames sets the name in the server app.  If this
	// was not called, pbrush would display a string with a bunch
	// of % sings in it.

	ret = OleSetHostNames( lpObject, GetApplication()->Name, lpszObjectName );
	wait( ret , lpObject );
}

void TOleDocWindow::CMUndo( RTMessage msg)
{
	if ( lpUndoObject )		// we have a previous object
		if ( lpUndoObject != lpObject )	// it is different then the current one
		{
			LPOLEOBJECT lpObjectToDelete = lpObject;
			lpObject = lpUndoObject;
			lpUndoObject = NULL;
			ret = OleDelete( lpObjectToDelete );
			wait( ret , lpObjectToDelete );
			bObjectLoaded = bUndoObjectLoaded;
			WMURedraw( msg );
		}

}

void TOleDocWindow::CMCut( RTMessage msg)
{
	CMCopy( msg );
	CloseCurrentOle();
}

void TOleDocWindow::CMCopy( RTMessage )
{
	if ( OpenClipboard( HWindow ) && EmptyClipboard() )
	{
		ret = OleCopyToClipboard( lpObject );
		check( ret );
		CloseClipboard();
	}
}

void TOleDocWindow::BackupObject()
{
	if ( lpObject )
	{
		ret = OleClone( lpObject, (LPOLECLIENT)pOwlClient,
				lhClientDoc, GetApplication()->Name, &lpUndoObject );
		wait( ret, lpObject );
		lstrcpy( lpszLastObjectName, lpszObjectName );
		lstrcpy( lpszObjectName , GetNextObjectName() );
		bUndoObjectLoaded = bObjectLoaded;
	}
}


void TOleDocWindow::CMPaste( RTMessage )
{
	if ( OpenClipboard( HWindow ) )
	{
		BackupObject();

		lstrcpy( lpszObjectName, GetNextObjectName() );

		ret = OleCreateFromClip( "StdFileEditing",
			(LPOLECLIENT)pOwlClient,
			lhClientDoc,
			lpszObjectName,
			&lpObject,
			olerender_draw,
			0 );
		check( ret );

		ret = OleSetHostNames( lpObject, GetApplication()->Name, lpszObjectName );
		wait( ret , lpObject );

		bObjectLoaded = TRUE;

		CloseClipboard();
		PostMessage( HWindow , WM_U_REDRAW, 0, 0L );
	}

}

LPSTR TOleDocWindow::GetNextObjectName()
{
	static char buffer[ MAXPATH ];
	wsprintf( buffer, "object #%03d", nNextObjectNum++ );
	return buffer;
}

void TOleDocWindow::Paint( HDC hdc, PAINTSTRUCT _FAR &)
{
	LPOLEOBJECT lpObjectToDraw = NULL;

	if ( bObjectLoaded )
		lpObjectToDraw = lpObject;
	else if ( lpUndoObject )
		lpObjectToDraw = lpUndoObject;

	if ( lpObjectToDraw ) {
		RECT rect;
		GetClientRect( HWindow, &rect );

		// Tips for OleDraw:
		// OleDraw will return OLE_ERROR_OBJECT if the
		// object to be drawn is invalid.  In a case like this,
		// that could happen if the Paint function does not get
		// informed about the object being deleted, or not having
		// yet been created.
		// You can also get OLE_ERROR_BLANK if you draw an object
		// which has not been stored.  A time when this could happen
		// is trying use OleDraw on the object after using OleCreate
		// to make it, but not yet using the Servers update command
		// to save the object with Ole.
		ret = OleDraw( lpObjectToDraw , hdc, &rect , NULL, 0 );
		wait( ret, lpObjectToDraw );
	}
}

TOleDocWindow::TOleDocWindow( PTWindowsObject parent, LPSTR title )
	: TWindow( parent, title )
{
	ret = OLE_OK;
	lhClientDoc = 0;

	bObjectLoaded = FALSE;
	bUndoObjectLoaded = FALSE;
	bUndoObjectLoaded = FALSE;

	pOwlClient = NULL;
	lpObject = NULL;
	lpUndoObject = NULL;

	strcpy( lpszDocName , "noname.ole" );
	*lpszLastObjectName = 0;
	*lpszObjectName = 0;
	bDefDocName = TRUE;
}

void TOleDocWindow::SetupWindow() {
	TWindow::SetupWindow();
	RegisterClientDoc();
	pOwlClient = new TOwlClient( this );

}


void TOleDocWindow::RegisterClientDoc() {
	// Tip for using OleRegisterClientDoc:
	// if the lpszDoc parameter passed to
	// OleRegisterClientDoc is not valid, (lpszPatnName in this case),
	// one can get an OLE_ERROR_ADDRESS error, and the ClientDoc
	// handle will not be valid.

	ret = OleRegisterClientDoc(
			GetApplication()->Name,
			lpszDocName,
			0,
			&lhClientDoc );
	check( ret );
}


void TOleDocWindow::ShutDownWindow()
{
	CloseCurrentOle();
	if ( pOwlClient ) delete pOwlClient;
	TWindow::ShutDownWindow();
}


void TOleDocWindow::RegFileName( LPSTR FileName )
{
	lstrcpy( lpszDocName , FileName );
	ret = OleRegisterClientDoc( GetApplication()->Name,
			lpszDocName ,
			0,
			&lhClientDoc );
	check ( ret );
}

void TOleDocWindow::CMActivate( RTMessage )
{
	BackupObject();
	RECT rect;
	GetClientRect( HWindow, &rect );
	ret = OleActivate( lpObject , OLEVERB_PRIMARY, TRUE, TRUE ,
			HWindow , &rect );
	wait ( ret, lpObject );
	PostMessage( HWindow , WM_U_REDRAW, 0, 0L );
}

void TOleDocWindow::WMInitMenu( RTMessage msg )
{
	HMENU hMenu = (HMENU)msg.WParam;
	WORD wEnableUndo;

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

	DrawMenuBar( HWindow );
}

LPSTR TOleDocWindow::GetClassName() { return "OLEDOCWINDOW"; }

void TOleDocWindow::GetWindowClass(WNDCLASS _FAR &wc )
{
	TWindow::GetWindowClass( wc );
	wc.lpszMenuName = "MENU_DOCWINDOW";
}

void TOleDocWindow::CMClear( RTMessage )
{
	CloseCurrentOle();
}

void TOleDocWindow::CloseCurrentOle()
{
	// assume saved
	if ( lpObject ) {
		ret = OleDelete( lpObject );
		wait( ret , lpObject );
	}
	if ( lpUndoObject ) {
		ret = OleDelete( lpUndoObject );
		wait( ret , lpObject );
	}
	lpObject = lpUndoObject = NULL;

	bObjectLoaded = FALSE;

	InvalidateRect( HWindow , NULL, TRUE );
	UpdateWindow( HWindow );
}


void TOleApp::InitMainWindow()
{
  MainWindow = new TOleDocWindow(NULL, "OWL OLE Application" );
}



int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   LPSTR lpCmd, int nCmdShow)
{
	TOleApp OleApp ("OleApp", hInstance, hPrevInstance,
		lpCmd, nCmdShow);
	OleApp.Run();
	return (OleApp.Status);
}

// General Help:
// Question: Right after closing down the server, the only part of the
//   object repainted in my application is that portion of the window that
//   was covered with the server program, whats going on?
// Answer: When the server closes, Windows sends you a paint message to
//	 update that portion of your window which the server covered up.  It
//	 has a clipping region defined which keeps all the previous visible
//	 portions of your client window.  To repaint the whole window after
//	 a server closes, respond to the OLE_CHANGED notification in your
//	 callback, and force the whole window to be updated there.  Calling
//	 InvalidateRect would be appropiate.

