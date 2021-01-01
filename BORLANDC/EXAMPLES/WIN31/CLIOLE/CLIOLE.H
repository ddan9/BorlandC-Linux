// Windows 3.1 Demo Program
// Copyright (c) 1992 by Borland International

// declares structures and functions used in the example.


// prototype for the standard Call back function used for Ole notifications.
// it will route notifications on to the appropiate object.

int FAR PASCAL _export StdCallBack( LPOLECLIENT , OLE_NOTIFICATION , LPOLEOBJECT );

// typdef for an OLE callback function
typedef int   FAR PASCAL ( *TCallBack )( LPOLECLIENT , OLE_NOTIFICATION , LPOLEOBJECT );


// struct: TOleClient
// This is an OLECLIENT structure with an HWND for the window which
// owns this object.
// An OLECLIENT's primary purpose is to provide the CallBack function
// which the Ole librarys will call to give notifications about a
// particular object.  The purpose of a TOleClient instance is to send those
// notification to the window which is managing that object.
// It will have no knowledge of the actual object which it receives
// notifications for.
// The OLECLIENT
// structure has a vtable which has the address of the callback function.
// since only one such vtable need be constructed for instances of this
// structure, a global variable will be used to determine if a vtable has
// been constructed.

typedef struct {
	OLECLIENT oleClient;
	HWND hwnd;
} TOleClient;



// user defined messages

#define WM_U_REDRAW 		(WM_USER + 1)
#define WM_U_CALLBACK    ( WM_USER + 2)

// structure used to pass Ole notification messages from the
// Ole Callback to the window.

typedef struct stagTOleNot {
	LPOLECLIENT lpClient;
	OLE_NOTIFICATION not;
	LPOLEOBJECT lpObject;
} TOleNot;

// function prototypes

long FAR PASCAL _export WndProc (HWND, WORD, WORD, LONG) ;
void WMURedraw( HWND hwnd );
void CMPBrush( );
void CMUndo(HWND);
void CMActivate( HWND hwnd );
void CMClear( HWND hwnd );
void CMCut(HWND hwnd);
void CMCopy( HWND hwnd );
void CMPaste( HWND hwnd );
void WMCreate( HWND hwnd );
void WMInitMenu( HWND hwnd , WORD wParam );
int WMUCallBack( LPOLECLIENT lpOleClient ,
				 OLE_NOTIFICATION oleNot,
				 LPOLEOBJECT lpOleObject );
void CMPBrush( );
void BackupObject();
LPSTR GetNextObjectName();
void CMUndo();
void CMCut(HWND hwnd );
void CMCopy( HWND hwnd );
void CloseCurrentOle( HWND hwnd );
void CMCopy( HWND hwnd );
void CMPaste( HWND hwnd );
void RegisterClientDoc();
void memerror();
void Paint ( HWND, HDC );
void CMAbout( HWND );
void InitApplication(void);
void InitInstance(void);
void CMPBrush(void);
void BackupObject(void);
LPSTR GetNextObjectName(void);
void RegisterClientDoc(void);
void memerror(void);








