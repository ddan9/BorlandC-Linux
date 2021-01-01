// (C) Copyright 1992 by Borland International
//
// Cmdlg.c - Common Dialogs example in C

/*********************************************************************
This program has example code that makes use of the Common Dialogs
available in Windows 3.1.  It is meant to be a simple example using the
C programming language.

The main window will have menu selections for opening a file, changing the
font and changing the color used for the selected font.  When a file is
selected the name will be displayed on the client area of the window.

The files needed to build this program are . . .

cmdlg.h					Header file for application
cmdlgr.h				   Header file for application and resources
cmdlg.c					Source file for application
cmdlg.rc				   Resource file for application
***********************************************************************/

// Contains the necessary headers and definitions for this program
#include "cmdlg.h"

/***  Global Variables  ***/
char szName[256];
COLORREF crColor;
HFONT hfFont;
BOOL tfFontLoaded;
HINSTANCE hInst;	// current instance


/*********************************************************************
Using the OPENFILENAME structure and the Windows 3.1 API call
GetOpenFileName() eases the selection of files for the programmer and for
the user.  The WINHELP.EXE help file WIN31WH.HLP (found in the BORLANDC\BIN
directory) contains a detailed description of the function call and its
associated structure.  The Flags field of the structure is particularly
useful when custimization is required.
**********************************************************************/
void CMUFileOpen( HWND hWnd )
{
	OPENFILENAME ofnTemp;
	DWORD Errval;	// Error value
	char buf[5];	// Error buffer
	char Errstr[50]="GetOpenFileName returned Error #";
	char szTemp[] = "All Files (*.*)\0*.*\0Text Files (*.txt)\0*.txt\0";
/*
Note the initialization method of the above string.  The GetOpenFileName()
function expects to find a string in the OPENFILENAME structure that has
a '\0' terminator between strings and an extra '\0' that terminates the
entire filter data set.  Using the technique shown below will fail because
"X" is really 'X' '\0' '\0' '\0' in memory.  When the GetOpenFileName()
function scans szTemp it will stop after finding the extra trailing '\0'
characters.

	char szTemp[][4] = { "X", "*.*", "ABC", "*.*", "" };

The string should be "X\0*.*\0ABC\0*.*\0".

Remember that in C or C++ a quoted string is automatically terminated with
a '\0' character so   char "X\0";   would result in 'X' '\0' '\0' which
provides the extra '\0' that GetOpenFileName() needs to see in order to
terminate the scan of the string.  Just 'char ch "X";' would result in 'X'
'\0' and GetOpenFileName() would wander off in memory until it lucked into
a '\0' '\0' character sequence.
*/

/*
Some Windows structures require the size of themselves in an effort to
provide backward compatibility with future versions of Windows.  If the
lStructSize member is not set the call to GetOpenFileName() will fail.
*/
	ofnTemp.lStructSize = sizeof( OPENFILENAME );
	ofnTemp.hwndOwner = hWnd;	// An invalid hWnd causes non-modality
	ofnTemp.hInstance = 0;
	ofnTemp.lpstrFilter = (LPSTR)szTemp;	// See previous note concerning string
	ofnTemp.lpstrCustomFilter = NULL;
	ofnTemp.nMaxCustFilter = 0;
	ofnTemp.nFilterIndex = 1;
	ofnTemp.lpstrFile = (LPSTR)szName;	// Stores the result in this variable
	ofnTemp.nMaxFile = sizeof( szName );
	ofnTemp.lpstrFileTitle = NULL;
	ofnTemp.nMaxFileTitle = 0;
	ofnTemp.lpstrInitialDir = NULL;
	ofnTemp.lpstrTitle = "OPENFILENAME";	// Title for dialog
	ofnTemp.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
	ofnTemp.nFileOffset = 0;
	ofnTemp.nFileExtension = 0;
	ofnTemp.lpstrDefExt = "*";
	ofnTemp.lCustData = NULL;
	ofnTemp.lpfnHook = NULL;
	ofnTemp.lpTemplateName = NULL;
/*
If the call to GetOpenFileName() fails you can call CommDlgExtendedError()
to retrieve the type of error that occured.
*/
	if(GetOpenFileName( &ofnTemp ) != TRUE)
	{
		Errval=CommDlgExtendedError();
		if(Errval!=0)	// 0 value means user selected Cancel
		{
			sprintf(buf,"%ld",Errval);
			strcat(Errstr,buf);
			MessageBox(hWnd,Errstr,"WARNING",MB_OK|MB_ICONSTOP);
		}

	}
	InvalidateRect( hWnd, NULL, TRUE );	// Repaint to display the new name
}

/*************************************************************************
Using the CHOOSECOLOR structure and the Windows 3.1 API call ChooseColor(),
eases the selection of colors for the programmer and for the user.  The
comments for the File Open dialog regarding the help file and the structure
size also apply to the color dialog.
**************************************************************************/
void CMUColor( HWND hWnd )
{
	CHOOSECOLOR ccTemp;
	COLORREF crTemp[16];	// Important, sometimes unused, array

	ccTemp.lStructSize = sizeof( CHOOSECOLOR );
	ccTemp.hwndOwner = hWnd;
	ccTemp.hInstance = 0;
	ccTemp.rgbResult = crColor;	// CC_RGBINIT flag makes this the default color
/*
lpCustColors must be set to a valid array of 16 COLORREF's, even if it
is not used.  If it isn't you will probably fail with a GP fault in
COMMDLG.DLL.
*/
	ccTemp.lpCustColors = crTemp;
	ccTemp.Flags = CC_PREVENTFULLOPEN | CC_RGBINIT;
	ccTemp.lCustData = 0L;
	ccTemp.lpfnHook = NULL;
	ccTemp.lpTemplateName = NULL;
	if( ChooseColor( &ccTemp ) == TRUE ) crColor = ccTemp.rgbResult;
	InvalidateRect( hWnd, NULL, TRUE );
}

/**************************************************************************
Using the CHOOSEFONT structure and the Windows 3.1 API call ChooseFont()
eases the selection of fonts for the programmer and for the user.  The
comments for the File Open dialog regarding the help file and the structure
size also apply to the font dialog.
***************************************************************************/
void CMUFont( HWND hWnd )
{
/*
The variables below are static so that multiple calls to the font dialog will
retain previous user selections.
*/
	static CHOOSEFONT cfTemp;
	static LOGFONT lfTemp;

	if( tfFontLoaded == TRUE )	// cfTemp contains previous selections
	{
		cfTemp.Flags |= CF_INITTOLOGFONTSTRUCT;
		cfTemp.rgbColors = crColor;
	}
	else
	{
		cfTemp.lStructSize = sizeof( CHOOSEFONT );
		cfTemp.hwndOwner = hWnd;
		cfTemp.hDC = 0;
		cfTemp.lpLogFont = &lfTemp;	// Store the result here
		cfTemp.Flags = CF_EFFECTS | CF_FORCEFONTEXIST | CF_SCREENFONTS;
		cfTemp.rgbColors = crColor;	// Color and font dialogs use the same color
		cfTemp.lCustData = 0L;
		cfTemp.lpfnHook = NULL;
		cfTemp.lpTemplateName = NULL;
		cfTemp.hInstance = 0;
		cfTemp.lpszStyle = NULL;
		cfTemp.nFontType = SCREEN_FONTTYPE;
		cfTemp.nSizeMin = 0;
		cfTemp.nSizeMax = 0;
	}
	if( ChooseFont( &cfTemp ) == TRUE )
	{
		if( tfFontLoaded == TRUE )
			DeleteObject( hfFont );
		crColor = cfTemp.rgbColors;
		hfFont = CreateFontIndirect( &lfTemp );
		tfFontLoaded = TRUE;
	}
	InvalidateRect( hWnd, NULL, TRUE );
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
	wc.lpszMenuName = "CMDLGAPMENU";	// Name of menu resource in .RC file.
	wc.lpszClassName = "CMDLG";	// Name used in call to CreateWindow.

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
		"CMDLG", 	               // See RegisterClass() call.
		"Common Dialog Example",	// Text for window title bar.
		WS_OVERLAPPEDWINDOW,	      // Window style.
		CW_USEDEFAULT, 			   // Default horizontal position.
		CW_USEDEFAULT,	   	   	// Default vertical position.
		CW_USEDEFAULT,	      		// Default width.
		CW_USEDEFAULT, 		   	// Default height.
		NULL,		   		         // Overlapped windows have no parent.
		NULL,	   			         // Use the window class menu.
		hInstance,  			      // This instance owns this window.
		NULL			   	         // Pointer not needed.
	);

	/* If window could not be created, return "failure" */

	if (!hWnd)
		return (FALSE);

	/* Make the window visible; update its client area; and return "success" */

	ShowWindow(hWnd, nCmdShow);	// Show the window
	UpdateWindow(hWnd);			// Sends WM_PAINT message
	return (TRUE);				// Returns the value from PostQuitMessage

}


/****************************************************************************
	FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)
****************************************************************************/
long FAR PASCAL _export MainWndProc(HWND hWnd, unsigned message,
									WORD wParam, LONG lParam)
{
	HFONT fTemp;	// Placeholder for the original font
	RECT rTemp;     // Client are needed by DrawText()
	HDC hdc;        // HDC for Window
	PAINTSTRUCT ps;	// Paint Struct for BeginPaint call

	switch (message) {
	case WM_CREATE: // Initialize Global vars
			strcpy( szName, "" );           // Empty the file name string
			crColor = RGB( 0, 0, 0 );       // Use black as the default color
			hfFont = 0;                     // Empty the handle to the font
			tfFontLoaded = FALSE;           // Set the font selected flag to false
		return NULL;

	case WM_PAINT:
	// Display the file name using the selected font in the selected color.

			hdc=BeginPaint(hWnd,&ps);
			SetTextColor( hdc, crColor );
			if( tfFontLoaded == TRUE )
				fTemp = (HFONT)SelectObject( hdc, hfFont );
			GetClientRect( hWnd, &rTemp );
			DrawText( hdc, szName, strlen( szName ), &rTemp, DT_CENTER | DT_WORDBREAK );
			if( tfFontLoaded == TRUE )
				SelectObject( hdc, fTemp );
			EndPaint(hWnd,&ps);
		break;

	case WM_COMMAND:	// message: command from application menu
		switch(wParam)
		{
			case CM_EXIT:
					DestroyWindow(hWnd);
				break;

			case CM_U_FILEOPEN:
					CMUFileOpen(hWnd);
				break;

			case CM_U_COLOR:
					CMUColor(hWnd);
				break;

			case CM_U_FONT:
					CMUFont(hWnd);
				break;

			case CM_U_HELPABOUT:
					MessageBox(hWnd,szCMDLGAPAbout,"About CMDLG",MB_OK);
				break;

			default:
				break;
		}
		break;

	case WM_QUIT:
	case WM_DESTROY:	// message: window being destroyed
			if( hfFont != 0 )
				DeleteObject( hfFont );
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
	MSG msg;			// message
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

// End of file cmdlg.c
