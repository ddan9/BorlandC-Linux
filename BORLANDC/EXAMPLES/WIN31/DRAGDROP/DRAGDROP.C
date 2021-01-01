// (C) Copyright 1992 by Borland International
//
// DragDrop.c - Drag and Drop example in C

// Beginning of file dragdrop.c

/* --------------------------------------------------------------
	This is a code example using the Drag and Drop feature in
	Windows 3.1 with C.  The example shows how to drop files
	into the client area of the main window and then print out
	the file names only if they were dropped in the client area.
	The list of files will be kept in a linked list of structures.
	The code is well commented with Steps to follow the creation
	of the application as well as comments for common pitfalls
	and important lines of code that affect the performance of
	your application.
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

#define STRICT	// define STRICT to provide maximum type safety
					// as defined by Microsoft

#include <windows.h>
#include <string.h>
#include <alloc.h>
/***************************************************************
	Step 0:
	This is the header file which declares the Drag and Drop
	functions. The functions are in SHELL.DLL
****************************************************************/
#include <shellapi.h>

long FAR PASCAL _export MainWndProc(HWND, unsigned, WORD, LONG);
void AddFiles(HWND, struct List*);

// Definition of structure types for Linked list of files
struct FileList {
	char *lpFileName;	// Name of dragged file
	int x, y;			// Position in client area dropped
	BOOL inClient;		// Dropped in client Flag
	struct FileList *Next;	// Points to next file in group dragged
};
struct List {
	struct List *Next;		// Points to next node in List
	struct FileList *FLptr;	// Points to Filelist(s)
};

// Global variables
HINSTANCE hInst;	// current instance
struct List *liAllFiles;	// Pointer to main list of files
BOOL GetHelp;


/********************************************************************/
void FileDrop( struct FileList* ptr, char *FileName , int px, int py,
				BOOL inClientarea )
{
	ptr->lpFileName =(char*)malloc(strlen( FileName ) + 1 );
	strcpy( ptr->lpFileName, FileName );
	ptr->x = px;
	ptr->y = py;
	ptr->inClient = inClientarea;
	ptr->Next=NULL;
}

char *WhoAmI(struct FileList *ptr)
{
	static char buffer[ 80 ];

	wsprintf( buffer, "File: %s [ %d , %d ] inClient: %d",
			 (LPSTR)ptr->lpFileName, ptr->x , ptr->y, ptr->inClient );

//WARNING!!:  Serious problem recognized:
		//In the small model, without the typecast,
		//lpFileName was two bytes on the stack of which
		//wsprintf took 4.

	return buffer;
}


void WMDropFiles( WORD WParam, HWND hWnd)
/*********************************************************
	Step 3:
	Retrieve a handle to an internal data structure in
	SHELL.DLL.  Get the info of the files out of it.
**********************************************************/
{
	struct FileList *FLptr=NULL, *FLCurrent=NULL, *FLHead=NULL;
	struct List *liFiles;
	int TotalNumberOfFiles;
	int nFileLength;
	char *pszFileName;
	POINT pPoint;
	BOOL inClientArea;
	int i;
/********************************************************
	Step 4:
	Identify the handle
*********************************************************/
	HDROP hDrop = (HDROP)WParam;
				//msg.WParam contains the Handle to the
				//internal data structure which has
				//information about the dropped file(s)

/****************************************************************
	Step 5:
	Find out total number of files dropped, by passing 0xFFFF for
	index parameter to DragQueryFile
*****************************************************************/
	TotalNumberOfFiles = DragQueryFile( hDrop , 0xFFFF, NULL, 0 );
	GetHelp=FALSE;

	liFiles =(struct List*)malloc(sizeof(struct List));

	for ( i = 0; i < TotalNumberOfFiles ; i++ )
	{
/************************************************************
	Step 6:
	Get the length of a filename by telling DragQueryFile
	which file your querying about ( i ), and passing NULL
	for the buffer parameter.
*************************************************************/
		nFileLength  = DragQueryFile( hDrop , i , NULL, 0 );
		pszFileName = (char*)malloc(nFileLength + 1);

/************************************************************
	Step 7:
	Copy a file name.   Tell DragQueryFile the file
	your interested in (i) and the length of your buffer.
	NOTE: Make sure that the length is 1 more then the filename
	to make room for the null charater!
*************************************************************/
		DragQueryFile( hDrop , i, pszFileName, nFileLength + 1 );
			//copies over the filename


/**************************************************************
	Step 8:
	Getting the file dropped. The location is relative to your
	client coordinates, and will have negative values if dropped
	in the non client parts of the window.

	DragQueryPoint copies that point where the file was dropped
	and returns whether or not the point is in the client area.
	Regardless of whether or not the file is dropped in the client
	or non-client area of the window, you will still receive the
	file name.
***************************************************************/

		inClientArea = DragQueryPoint( hDrop , &pPoint );
		FLptr= (struct FileList*)malloc(sizeof(struct FileList));
		if(FLHead == NULL)
			FLHead = FLptr;

		FileDrop(FLptr, pszFileName , pPoint.x, pPoint.y , inClientArea );

		// Add new struct to FileList structure list
		if(FLHead != FLptr)
		{
			FLCurrent->Next=FLptr;
			FLCurrent=FLptr;
			FLptr->Next=NULL;
		}
		else
		{
			FLHead->Next=NULL;
			FLCurrent = FLHead;
		}
	}
	liFiles->FLptr=FLHead;	// Point to first FileList struct in list
	AddFiles( hWnd, (struct List*)liFiles );  //Add this sublist of dropped files
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

/************************************************************/
void AddFiles( HWND hWnd, struct List *pliFiles )
{
	if(liAllFiles==NULL)
	{
		liAllFiles=pliFiles;
		liAllFiles->Next=NULL;
	}
	else
	{
		pliFiles->Next=liAllFiles;  // Put at front of list
		liAllFiles=pliFiles;
	}
	InvalidateRect( hWnd, NULL, TRUE );
	UpdateWindow( hWnd );
}

/************************************************************
	Delete FileList structures on linked list
*************************************************************/
void deleteFileList( struct FileList *pFList)
{
	if(pFList->Next != NULL)
		deleteFileList(pFList->Next);   // Recursion - delete next FileList struct
	free(pFList->lpFileName);       // free string allocation
	free(pFList);                   // free structure FileList
}

/************************************************************
	Delete List structures on linked list
*************************************************************/
void deleteList( struct List *pliFiles)
{
	if(pliFiles != NULL)
	{
		if(pliFiles->Next != NULL)
			deleteList(pliFiles->Next);     // Recursion - delete next List struct
		if(pliFiles->FLptr != NULL)
			deleteFileList(pliFiles->FLptr);// Initial call to free FileList
		free(pliFiles);             // free List struct
	}
}


/**********************************************************************/
BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS  wc;

	// Fill in window class structure with parameters that describe the
	// main window.

	wc.style = CS_HREDRAW | CS_VREDRAW;	// Class style(s).
	wc.lpfnWndProc = (long (FAR PASCAL*)())MainWndProc;	// Function to retrieve messages for
														// windows of this class.
	wc.cbClsExtra = 0;	// No per-class extra data.
	wc.cbWndExtra = 0;	// No per-window extra data.
	wc.hInstance = hInstance;	// Application that owns the class.
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = "DRAGDROPMENU";
	wc.lpszClassName = "DragnDrop"; // Name used in call to CreateWindow.

	/* Register the window class and return success/failure code. */

	return (RegisterClass(&wc));
}


/************************************************************************/
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND	hWnd;	// Main window handle.

	/* Save the instance handle in static variable, which will be used in  */
	/* many subsequence calls from this application to Windows.            */

	hInst = hInstance;

	/* Create a main window for this application instance.  */

	hWnd = CreateWindow(
		"DragnDrop",	// See RegisterClass() call.
		"Drag & Drop Example",	// Text for window title bar.
		WS_OVERLAPPEDWINDOW,	// Window style.
		CW_USEDEFAULT,			// Default horizontal position.
		CW_USEDEFAULT,			// Default vertical position.
		CW_USEDEFAULT,			// Default width.
		CW_USEDEFAULT,			// Default height.
		NULL,				// Overlapped windows have no parent.
		NULL,				// Use the window class menu.
		hInstance,			// This instance owns this window.
		NULL				// Pointer not needed.
	);

	/* If window could not be created, return "failure" */

	if (!hWnd)
		return (FALSE);

	/* Make the window visible; update its client area; and return "success" */

	ShowWindow(hWnd, nCmdShow);	// Show the window
	UpdateWindow(hWnd);		// Sends WM_PAINT message
	return (TRUE);		// Returns the value from PostQuitMessage

}


/****************************************************************************
	FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)
****************************************************************************/
long FAR PASCAL _export MainWndProc(HWND hWnd, unsigned message,
					 WORD wParam, LONG lParam)
{
	HDC hdc;		// HDC for Window
	PAINTSTRUCT ps;	// Paint Struct for BeginPaint call
	RECT rect;
	struct List* Ltemp;
	struct FileList* FLtemp;
	char *bufstring;
	int i;

	switch (message) {
	case WM_CREATE:	// Initialize Global vars
			GetHelp=TRUE;
/************************************************************
	liAllFiles is a pointer to a linked list of structure
	List.  Each List in the linked list will contain the file
	names that were received during a WM_DROPFILES messages
	and will be stored in a linked list of structure FileList.
*************************************************************/
			liAllFiles=NULL;

/****************************************************************
	Step 1:
	calling DragAcceptFiles.  If you pass FALSE, you're saying
	I don't accept them anymore.
*****************************************************************/
			DragAcceptFiles( hWnd , TRUE );

		return NULL;
	case WM_DROPFILES:
/************************************************************
	Step 2:
	Make call to WMDropFiles
*************************************************************/
			WMDropFiles(wParam, hWnd);
		break;
	case WM_PAINT:
	// Display the file name(s)
			hdc=BeginPaint(hWnd,&ps);
			if(GetHelp)
			{
				GetClientRect(hWnd,&rect);
				DrawText(hdc,HelpText,strlen(HelpText),&rect,DT_CENTER|DT_WORDBREAK);
			}
			else
			{
				SetBkMode ( hdc , TRANSPARENT );
				Ltemp=liAllFiles;

				i=0;
				while ( Ltemp != NULL )
				{
					FLtemp=Ltemp->FLptr;
					while( FLtemp != NULL )
					{
						bufstring = WhoAmI(FLtemp);
						TextOut( hdc, 10, 20 * i++, bufstring, strlen(bufstring));
						FLtemp=FLtemp->Next;
					}
					i++;
					Ltemp=Ltemp->Next;
				}
			}
			EndPaint(hWnd,&ps);
		break;
	case WM_COMMAND:	// message: command from application menu
		switch(wParam)
		{
			case 101:
				GetHelp=TRUE;
				InvalidateRect(hWnd,NULL,TRUE);
				UpdateWindow(hWnd);
			break;
			case 102:
				GetHelp=FALSE;
				InvalidateRect(hWnd,NULL,TRUE);
				UpdateWindow(hWnd);
			break;
			case 103:
				GetHelp=FALSE;
				deleteList(liAllFiles);
				liAllFiles=NULL;
				InvalidateRect(hWnd,NULL,TRUE);
				UpdateWindow(hWnd);
			break;
			case 104:
                MessageBox(hWnd,"DRAG n' DROP Example\nCopyright (c) 1992 Borland International, Inc.","ABOUT BOX",MB_OK);
			break;

			default:
			break;
		}
		break;
	case WM_CLOSE:
/****************************************************************
	Step 10:
	Don't accept files anymore
*****************************************************************/
			DragAcceptFiles( hWnd , FALSE );

			deleteList(liAllFiles);
			DestroyWindow(hWnd);
		break;
	case WM_QUIT:
	case WM_DESTROY:	// message: window being destroyed
			PostQuitMessage(0);
		break;
	default:			// Passes it on if unproccessed
		return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	return (NULL);
}

#pragma argsused
/**************************************************************/
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
			 LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;		// message
	if (!hPrevInstance)	// Other instances of app running?
	if (!InitApplication(hInstance))	// Initialize shared things
		return (FALSE);	// Exits if unable to initialize

	/* Perform initializations that apply to a specific instance */

	if (!InitInstance(hInstance, nCmdShow))
		return (FALSE);

	/* Acquire and dispatch messages until a WM_QUIT message is received. */

	while (GetMessage(&msg,	// message structure
		NULL,	// handle of window receiving the message
		NULL,	// lowest message to examine
		NULL))	// highest message to examine
	{
		TranslateMessage(&msg);	// Translates virtual key codes
		DispatchMessage(&msg);	// Dispatches message to window
	}
	return (msg.wParam);	// Returns the value from PostQuitMessage
}

// end of file dragdrop.c
