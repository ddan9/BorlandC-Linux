// ObjectWindows - (C) Copyright 1992 by Borland International
//
// Cmdlgap.cpp - Common Dialogs example in OWL

// Beginning of file cmdlgap.cpp

/*
This program has example code that makes use of the Common Dialogs available
in Windows 3.1.  It is meant to be a simple example, which is why there are
no classes derived from TDialog for each of the Common Dialogs.  The code
that actually brings up the Common Dialogs can be easily ported to Windows
applications written in C.

The main window will have menu selections for opening a file, changing the
font and changing the color used for the selected font.  When a file is
selected the name will be displayed on the client area of the window.

The files needed to build this program are . . .

cmdlgap.h				Header file for application
cmdlgapr.h				Header file for application and resources
cmdlgap.cpp				Source file for application
cmdlgap.rc				Resource file for application
*/

#include "cmdlgap.h"

TCMDLGApp::TCMDLGApp( LPSTR lpszNam, HINSTANCE hInst, HINSTANCE hPrevInst,
	LPSTR lpszCmdLn, int nCmdShw )
	: TApplication( lpszNam, hInst, hPrevInst, lpszCmdLn, nCmdShw )
{
}

TCMDLGApp::~TCMDLGApp()
{
}

void TCMDLGApp::InitMainWindow()
{
	MainWindow = new TCMDLGWnd( NULL, "Common Dialog example" );
}

TCMDLGWnd::TCMDLGWnd( PTWindowsObject AParent, LPSTR ATitle )
	: TWindow( AParent, ATitle )
{
/*
The OPENFILENAME, CHOOSECOLOR and CHOOSEFONT structures could be members of
this OWL window class.  If the initialization were done in this constructor
the HWindow data member _would not_ contain the appropriate value.  A better
place for the initialization of structures requiring an HWindow would be a
virtual SetupWindow() member function.
*/
	AssignMenu( "CMDLGAPMENU" );      // Set up the menu
	strcpy( szName, "" );           // Empty the file name string
	crColor = RGB( 0, 0, 0 );       // Use black as the default color
	hfFont = 0;                     // Empty the handle to the font
	tfFontLoaded = FALSE;           // Set the font selected flag to false
}

TCMDLGWnd::~TCMDLGWnd()
{
   if (hfFont)
      DeleteObject(hfFont);
}

LPSTR TCMDLGWnd::GetClassName()
{
	return "TCMDLGWnd";
}

// Display the file name using the selected font in the selected color.
void TCMDLGWnd::Paint( HDC hdc, PAINTSTRUCT _FAR & )
{
	HFONT fTemp;	// Placeholder for the original font
	RECT rTemp;		// Client are needed by DrawText()

	SetTextColor( hdc, crColor );
	if( tfFontLoaded == TRUE ) fTemp = (HFONT)SelectObject( hdc, hfFont );
	GetClientRect( HWindow, &rTemp );
	DrawText( hdc, szName, strlen( szName ), &rTemp, DT_CENTER | DT_WORDBREAK );
    if( tfFontLoaded == TRUE )
		SelectObject( hdc, fTemp );
}

void TCMDLGWnd::CMExit( RTMessage Msg )
{
	TWindow::CMExit( Msg );
}

/***************************************************************************
Using the OPENFILENAME structure and the Windows 3.1 API call
GetOpenFileName() eases the selection of files for the programmer and for
the user.  The WINHELP.EXE help file WIN31WH.HLP (found in the BORLANDC\BIN
directory) contains a detailed description of the function call and its
associated structure.  The Flags field of the structure is particularly
useful when custimization is required.
****************************************************************************/
void TCMDLGWnd::CMUFileOpen( RTMessage )
{
	OPENFILENAME ofnTemp;
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
	ofnTemp.hwndOwner = HWindow;			// An invalid hWnd causes non-modality
	ofnTemp.hInstance = 0;
	ofnTemp.lpstrFilter = (LPSTR)szTemp;	// See previous note concerning string
	ofnTemp.lpstrCustomFilter = NULL;
	ofnTemp.nMaxCustFilter = 0;
	ofnTemp.nFilterIndex = 1;
	ofnTemp.lpstrFile = (LPSTR)szName;		// Stores the result in this variable
	ofnTemp.nMaxFile = sizeof( szName );
	ofnTemp.lpstrFileTitle = NULL;
	ofnTemp.nMaxFileTitle = 0;
	ofnTemp.lpstrInitialDir = NULL;
	ofnTemp.lpstrTitle = Title;				// Title for dialog
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
		DWORD Errval;	// Error value
		char Errstr[50]="GetOpenFileName returned Error #";
		char buf[5];	// Error buffer

		Errval=CommDlgExtendedError();
		if(Errval!=0)   // 0 value means user selected Cancel
		{
			sprintf(buf,"%ld",Errval);
			strcat(Errstr,buf);
			MessageBox(HWindow,Errstr,"WARNING",MB_OK|MB_ICONSTOP);
		}
	}
	InvalidateRect( HWindow, NULL, TRUE );	// Repaint to display the new name
}

/***************************************************************************
Using the CHOOSECOLOR structure and the Windows 3.1 API call ChooseColor(),
eases the selection of colors for the programmer and for the user.  The
comments for the File Open dialog regarding the help file and the structure
size also apply to the color dialog.
****************************************************************************/
void TCMDLGWnd::CMUColor( RTMessage )
{
	CHOOSECOLOR ccTemp;
	COLORREF crTemp[16];	// Important, sometimes unused, array

	ccTemp.lStructSize = sizeof( CHOOSECOLOR );
	ccTemp.hwndOwner = HWindow;
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
    if( ChooseColor( &ccTemp ) == TRUE )
        crColor = ccTemp.rgbResult;
	InvalidateRect( HWindow, NULL, TRUE );
}

/***************************************************************************
Using the CHOOSEFONT structure and the Windows 3.1 API call ChooseFont()
eases the selection of fonts for the programmer and for the user.  The
comments for the File Open dialog regarding the help file and the structure
size also apply to the font dialog.
****************************************************************************/
void TCMDLGWnd::CMUFont( RTMessage )
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
		cfTemp.hwndOwner = HWindow;
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
	InvalidateRect( HWindow, NULL, TRUE );
}

void TCMDLGWnd::CMUHelpAbout( RTMessage )
{
    MessageBox(HWindow,szCMDLGAPAbout,"ABOUT BOX",MB_OK);
}

int PASCAL WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLn,
	int nCmdShw )
{
	TCMDLGApp App( "CMDLGAP", hInst, hPrevInst, lpszCmdLn, nCmdShw );

	App.Run();
	return App.Status;
}

// End of file cmdlgap.cpp

