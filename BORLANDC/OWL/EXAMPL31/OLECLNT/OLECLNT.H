// ObjectWindows - (C) Copyright 1992 by Borland International
//
// oleclnt.h

_CLASSDEF( TOleApp )
_CLASSDEF( TOwlClient )
_CLASSDEF( TOleDocWindow )
_CLASSDEF( TSelectObjectDialog )

// prototype for the standard Call back function used for Ole notifications.
// it will route notifications on to the appropiate object.
int FAR PASCAL _export StdCallBack( LPOLECLIENT , OLE_NOTIFICATION , LPOLEOBJECT );

// This is a typedef for the Call Back function which the
// Ole API uses.
typedef int   FAR PASCAL ( *TCallBack )( LPOLECLIENT , OLE_NOTIFICATION , LPOLEOBJECT );

class TOleApp : public TApplication {

public:
	TOleApp(LPSTR name, HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmd,
		  int nCmdShow)
		: TApplication(name, hInstance,
				   hPrevInstance, lpCmd, nCmdShow) {};
	virtual void InitMainWindow();
	virtual void InitInstance();

	OLECLIPFORMAT  vcfLink, vcfNative, vcfOwnerLink;
	 // These are clipboard formats that Ole
	 // applications use to determine the type of
	 // object on the clipboard.  This application does not
	 // make use of them.
};

// class: TOwlClient
// This is an OLECLIENT structure with a pointer to a TOleDocWindow.
// An OLECLIENT's primary purpose is to provide the CallBack function
// which the Ole librarys will call to give notifications about a
// particular object.  The purpose of a TOwlClient instance is to send those
// notification to the TOleDocWindow instance which constructed it.
// It will have no knowledge of the actual Object which it receives
// notifications for.
// The OLECLIENT
// structure has a vtable which has the address of the callback function.
// since only one such vtable need be constructed for instances of this
// class, a static data member is used to determine if a vtable has
// been constructed.

struct TOwlClient : OLECLIENT
{
	PTOleDocWindow TOleDocWindowThis;
	TOwlClient( PTOleDocWindow , HINSTANCE hInst = 0);
	static LPOLECLIENTVTBL lpClientVtbl;
};


// An Ole Document window, it responds to the Ole edit commands to put
// manipulate an object in the window.

#define WM_U_REDRAW 		(WM_USER + 1)

class TOleDocWindow : public TWindow {
protected:
	OLESTATUS ret;   //last OleStatus
	LHCLIENTDOC lhClientDoc;
	char lpszDocName[ MAXPATH ];
	BOOL bDefDocName;

	char lpszObjectName[ MAXPATH ];
	char lpszLastObjectName[ MAXPATH ];
	PTOwlClient pOwlClient;
	BOOL bObjectLoaded,  bUndoObjectLoaded ;
	LPOLEOBJECT lpObject;		// actual object we are housing
	LPOLEOBJECT lpUndoObject;	// last change made to object
	static int nNextObjectNum;
public:
	TOleDocWindow( PTWindowsObject , LPSTR  );
	virtual LPSTR GetClassName();
	virtual void GetWindowClass( WNDCLASS _FAR &);
	virtual void SetupWindow();
	virtual void Paint( HDC , PAINTSTRUCT _FAR &);

	virtual void RegisterClientDoc();
	void RegisterClient();
	virtual void ShutDownWindow();
	void CloseCurrentOle();
	void RegFileName( LPSTR );
	void BackupObject();

	virtual void WMURedraw( RTMessage ) = [ WM_FIRST + WM_U_REDRAW ];
	virtual void WMInitMenu( RTMessage ) = [ WM_FIRST + WM_INITMENU ];

	virtual void CMPBrush( RTMessage ) = [ CM_FIRST + CM_PBRUSH ];
	virtual void CMUndo( RTMessage )   = [ CM_FIRST + CM_UNDO ];
	virtual void CMCut( RTMessage )    = [ CM_FIRST + CM_CUT ];
	virtual void CMCopy( RTMessage )   = [ CM_FIRST + CM_COPY ];
	virtual void CMPaste( RTMessage )  = [ CM_FIRST + CM_PASTE ];
	virtual void CMClear( RTMessage )  = [ CM_FIRST + CM_CLEAR ];
	virtual void CMActivate( RTMessage)= [ CM_FIRST + CM_ACTIVATE ];
	virtual void CMAbout( RTMessage)   = [ CM_FIRST + CM_ABOUT ];

	LPSTR GetNextObjectName();

	int CallBack( LPOLECLIENT , OLE_NOTIFICATION , LPOLEOBJECT );
};
