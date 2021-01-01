// ObjectWindows - (C) Copyright 1992 by Borland International
//
// Dragdrop.cpp - Drag and Drop example in OWL

// Beginning of file dragdrp.cpp

/* --------------------------------------------------------------
	This is a code example using the Drag and Drop feature in
	Windows 3.1 with OWL.  The example shows how to drop files
	into the client area of the main window and then print
	out the file names that were dropped in the client area.
	TMyWindow maintains this information in a List of Lists.
	Each sub List is a set of files that were dropped.  The code
	is well commented with Steps to follow the creation of the
	application as well as comments for common pitfalls and
	important lines of code that affect the performance of your
	application.
 --------------------------------------------------------------- */

char HelpText[]=
"\n\rThis application must be run under Windows 3.1.\n\r \
Bring up the Windows 3.1 File Manager.  Select a file\n\r \
with the left mouse button and keep the button down.\n\r \
Now drag the mouse over until it is on top of the drag\n\r \
n' drop window client area.  Now release the left mouse\n\r \
button.  You have just dragged and dropped a file.  To\n\r \
drag a group of files, select the group with the Shift\n\r \
and arrow keys and then repeat the previous directions.";

//#define STRICT	// define STRICT to provide maximum type safety
					// as defined by Microsoft
					// Already defined in project and make file

//#define WIN31		// define WIN31 for Windows 3.1 features
					// Already defined in project and make file

#include <owl.h>
#include <string.h>
#include <list.h>
/***************************************************************
	Step 0:
	This is the header file which declares the Drag and Drop
	functions. The functions are in SHELL.DLL
****************************************************************/
#include <shellapi.h>

class TMyApp : public TApplication
{
public:
	TMyApp(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
	: TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow) {};
	virtual void InitMainWindow();
};

_CLASSDEF( TMyWindow );
class TMyWindow : public TWindow
{
	BOOL GetHelp;
protected:
	List *liAllFiles;	// liAllFiles is a pointer to a linked list.
						// Each Object in the list will also be a list.
						// These lists will contain the names of the
						// files that have been dragged & dropped in
						// a group.
public:
	TMyWindow( PTWindowsObject, LPSTR );
	virtual void SetupWindow();
	virtual void ShutDownWindow();
	virtual LPSTR GetClassName();
	virtual void GetWindowClass( WNDCLASS _FAR & );

/************************************************************
	Step 2:
	Dispatch a message to WM_DROPFILES
*************************************************************/
	virtual void WMDropFiles( RTMessage ) = [ WM_FIRST + WM_DROPFILES ];

	void AddFiles( List * );
	virtual void Paint( HDC , PAINTSTRUCT _FAR & );
	virtual void HowTo101( RTMessage msg)=[CM_FIRST + 101]
	{
		GetHelp=TRUE;
		InvalidateRect( HWindow, NULL, TRUE );
		UpdateWindow( HWindow );
	}
	virtual void View102( RTMessage msg)=[CM_FIRST + 102]
	{
		GetHelp=FALSE;
		InvalidateRect( HWindow, NULL, TRUE );
		UpdateWindow( HWindow );
	}
	virtual void Clear103( RTMessage msg)=[CM_FIRST + 103]
	{
		GetHelp=FALSE;
		delete liAllFiles;
		liAllFiles = new List;
		InvalidateRect( HWindow, NULL, TRUE );
		UpdateWindow( HWindow );
	}
	virtual void About104( RTMessage msg)=[CM_FIRST + 104]
	{
		MessageBox(HWindow,"DRAG n' DROP\nWritten using ObjectWindows\nCopyright (c) 1992 Borland",
					"ABOUT BOX",MB_OK);
	}

};

/************************************************************
 * liAllFiles of TMyWindow is a linked list of Lists.
 * Each List in the linked list will contain the file names
 * that were received during a WM_DROPFILES messages.
 * The following declarations are to build up the object
 * which will desribe an individual file that was droped
 * during one of these messages.
 * Since these objects are derived from Object, they
 * can be inserted into a List.
*************************************************************/
#define __NonAbstractObject  __firstUserClass

class NonAbstractObject : public Object
/************************************************************
	This is a base class to take care of redefining all the
	pure virtual functions that make Object abstract.
*************************************************************/

{
	virtual classType isA() const { return __NonAbstractObject; }
	virtual Pchar nameOf()  const { return "NonAbstractObject"; }
	virtual hashValueType hashValue() const { return ( hashValueType )this; }
	virtual int   isEqual(RCObject test)  const
		{ return this ==  &(NonAbstractObject &)test; }
	virtual void      	   printOn(Rostream outputStream) const
		{ outputStream << nameOf() << endl; }
};

class TFileDrop : public NonAbstractObject
/*************************************************************
	This class Maintains information about a dropped file,
	its name, where it was dropped, and whether or not it
	was in the client area
**************************************************************/
{
public:
	char *lpFileName;
	POINT pPoint;
	BOOL inClientArea;

	TFileDrop( char * , POINT &, BOOL );
	char * WhoAmI();
	~TFileDrop() { delete lpFileName; }
};


TFileDrop::TFileDrop( char *FileName , POINT &p, BOOL inClient )
{
	lpFileName = new char[  strlen( FileName ) + 1 ];
	strcpy( lpFileName, FileName );
	pPoint.x = p.x;
	pPoint.y = p.y;
	inClientArea = inClient;
}

char *TFileDrop::WhoAmI()
{
	static char buffer[ 80 ];

	wsprintf( buffer, "File: %s [ %d , %d ] inClient: %d",
			(LPSTR)lpFileName, pPoint.x , pPoint.y, inClientArea );

//WARNING!!:  Serious problem recognized:
			//In the small model, without the typecast,
			//lpFileName was two bytes on the stack of which
			//wsprintf took 4.

	return buffer;
}


void TMyWindow::WMDropFiles( RTMessage msg )
/*********************************************************
	Step 3:
	Retrieve a handle to an internal data structure in
	SHELL.DLL.  Get the info out of it.
**********************************************************/
{
/*********************************************************
	Step 4:
	Identify the handle
**********************************************************/
	HDROP hDrop = (HDROP)msg.WParam;
				//msg.WParam contains the Handle to the
				//internal data structure which has
				//information about the dropped file(s)

/* ************************************************************
	Step 5:
	Find out how many files are dropped,
	To find out total number of dropped files,
	pass 0xFFFF for index parameter to DragQueryFile
   ************************************************************/
	int TotalNumberOfFiles = DragQueryFile( hDrop , 0xFFFF, NULL, 0 );
	GetHelp=FALSE;

	List *liFiles = new List;

	for ( int i = 0; i < TotalNumberOfFiles ; i++ )
	{
/************************************************************
	Step 6:
	Get the length of a filename by telling DragQueryFile
	which file your querying about ( i ), and passing NULL
	for the buffer parameter.
*************************************************************/
		int nFileLength  = DragQueryFile( hDrop , i , NULL, 0 );

		char *pszFileName = new char [ nFileLength + 1];

/************************************************************
	Step 7:
	Copy a file name.   Tell DragQueryFile the file
	your interested in (i) and the length of your buffer.
	NOTE: Make sure that the length is 1 more then the filename
	to make room for the null charater!
*************************************************************/
		DragQueryFile( hDrop , i, pszFileName, nFileLength + 1 );
			//copies over the filename

		POINT pPoint ;
		BOOL inClientArea;

/**************************************************************
	Step 8:
	Getting the file dropped. The location is relative to your
	client coordinates, and will have negative values if dropped in
	the non client parts of the window.

	DragQueryPoint copies that point where the file was dropped
	and returns whether or not the point is in the client area.
	Regardless of whether or not the file is dropped in the client
	or non-client area of the window, you will still receive the
	file name.
***************************************************************/

		inClientArea = DragQueryPoint( hDrop , &pPoint );

		liFiles->add( * new TFileDrop( pszFileName , pPoint , inClientArea ));

	}
	AddFiles( liFiles );	//Add this sublist of dropped files
							//to the big list.
/************************************************************
	Step 9:
	Release the memory shell allocated for this handle
	with DragFinish.
	NOTE: This is a real easy step to forget and could
	explain memory leaks and incorrect program performance.
*************************************************************/
	DragFinish( hDrop );
}

void TMyWindow::AddFiles( List *pliFiles )
{
	liAllFiles->add( *pliFiles );
	InvalidateRect( HWindow, NULL, TRUE );
	UpdateWindow( HWindow );
}

TMyWindow::TMyWindow( PTWindowsObject p, LPSTR s ) : TWindow( p, s )
{
	liAllFiles = new List;
	GetHelp=TRUE;
}

void TMyWindow::Paint( HDC hdc, PAINTSTRUCT _FAR &)
{
	RECT rect;

	if(GetHelp)
	{
		GetClientRect(HWindow,&rect);
		DrawText(hdc, HelpText, strlen(HelpText),&rect,DT_NOCLIP|DT_CENTER|DT_WORDBREAK);
	}
	else
	{
		SetBkMode ( hdc , TRANSPARENT );

		// get a list iterator for the main list.
		ListIterator liMainList = ( ListIterator & )
		liAllFiles->initIterator();
		int i = 0;

		while ( liMainList != 0 )
		{
			// for each element in the main list, get a
			// list iterator for that element, which is itself
			// a list, call it a sub list

			ListIterator liSubList =  ( ListIterator & )
				((List &)( liMainList.current() )).initIterator();
			while ( liSubList != 0 )
			{
				// for each element in the sub list, get the string
				// which describes it, we know these elements are
				// instances of the TFileDrop class

				char *string = ((TFileDrop &)(liSubList.current())).WhoAmI();

				TextOut( hdc , 10 , 20 * i++ , string, strlen( string ) );
				liSubList++;
			}
			liMainList++;
			i++;
		}
	}
}

void TMyWindow::SetupWindow()
{
	TWindow::SetupWindow();
/****************************************************************
	Step 1:
	calling DragAcceptFiles.  If you  pass FALSE, you're saying
	I don't accept them anymore.  Notice how it takes an HWindow.
	WARNING: Don't do this in the constructor!  HWindow is NOT
	valid at that point.
*****************************************************************/

	DragAcceptFiles( HWindow , TRUE );
}

void TMyWindow::ShutDownWindow()
{
/****************************************************************
	Step 10:
	Don't accept files anymore
*****************************************************************/

	DragAcceptFiles( HWindow , FALSE );
	delete liAllFiles;
	TWindow::ShutDownWindow();
}

LPSTR TMyWindow::GetClassName()
{
	return "MYWINDOW";
}

void TMyWindow::GetWindowClass( WNDCLASS _FAR & wc )
{
	TWindow::GetWindowClass( wc );
	wc.lpszMenuName=(LPSTR)"DRAGMENU";
}

void TMyApp::InitMainWindow()
{
	MainWindow = new TMyWindow(NULL, Name);
}


int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	TMyApp MyApp("Drag & Drop Example", hInstance, hPrevInstance,
			lpCmdLine, nCmdShow);
	MyApp.Run();
	return MyApp.Status;
}

// end of file dragdrp.cpp
