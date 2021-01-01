// ObjectWindows - (C) Copyright 1992 by Borland International
//
// owlhelp.cpp

/* -------------------------------------------------------------
 This is an example of an application which implements context
 sensitive help for menu choices.  To use the application, hit
 F1 when a menu item is highlighted.  The program checks for F1
 being down in the WM_ENTERIDLE message.  If it is down, it sets
 a flag and simulates the selection of the menu item.  The help
 is then shown in the command message for that menu item.
 When the command is received, we just check to see if the flag
 has been set which indicates that the user wants help on the command.
 ---------------------------------------------------------------- */

#include <owl.h>

#include "owlhelpm.h"
#include "owlhelpr.h"

#include "owlhelp.h"

TOWLHELPApp::TOWLHELPApp( LPSTR lpszNam, HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLn, int nCmdShw )
	: TApplication( lpszNam, hInst, hPrevInst, lpszCmdLn, nCmdShw )
{
}

void TOWLHELPApp::InitInstance()
{
	TApplication::InitInstance();
	HAccTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( OWLHELPAPACCEL ) );
}

void TOWLHELPApp::InitMainWindow()
{
	MainWindow = new TOWLHELPWnd( NULL, "OWL Help Example" );
}

TOWLHELPWnd::TOWLHELPWnd( PTWindowsObject AParent, LPSTR ATitle )
	: TWindow( AParent, ATitle )
{
}

LPSTR lpszClientAreaText = "To Get help, press F1 while a menu item is \
highlighted.  WinHelp will be called to show help on that topic.  If help \
is not shown, it is because the mouse button is pressed.  Release the \
mouse button to see help.";


void TOWLHELPWnd::Paint( HDC hdc, PAINTSTRUCT _FAR &)
{
	RECT rectClient;
	GetClientRect( HWindow , &rectClient );

	int nWidth = rectClient.right;		// rectClient.left is 0
	int nHeight = rectClient.bottom;	// rectClient.top is 0

	rectClient.left += (nWidth / 4);
	rectClient.top  += (nHeight / 4);
	rectClient.right -= (nWidth / 4);

	SetBkMode( hdc, TRANSPARENT );
	DrawText( hdc, lpszClientAreaText, lstrlen( lpszClientAreaText ),
		&rectClient, DT_CENTER | DT_VCENTER | DT_WORDBREAK );
}

void TOWLHELPWnd::GetWindowClass( WNDCLASS &AWndClass )
{
	TWindow::GetWindowClass( AWndClass );
	AWndClass.lpszMenuName = MAKEINTRESOURCE( OWLHELPAPMENU );
}

LPSTR TOWLHELPWnd::GetClassName()
{
	return "TOWLHELPWnd";
}

void TOWLHELPWnd::SetupWindow()
{
	tfF1Pressed = FALSE;
}

void TOWLHELPWnd::WMEnterIdle( RTMessage Msg )
{
	if( (Msg.WParam == MSGF_MENU ) && ((GetKeyState( VK_F1 ) & 0x8000) != 0) )
		// if the high bit is set, then the key is pressed
	{
		tfF1Pressed = TRUE;
		PostMessage( HWindow, WM_KEYDOWN, VK_RETURN, 0L );
	}
}

void TOWLHELPWnd::CMUMenuItemA( RTMessage )
{
	if( tfF1Pressed == TRUE )
	{
		WinHelp( HWindow, "OWLHELP.HLP", HELP_CONTEXT, HELP_MENUITEMA );
		tfF1Pressed = FALSE;
	} else {
		MessageBox( HWindow, "In Menu Item A command", Title, MB_ICONINFORMATION );
	}
}

void TOWLHELPWnd::CMUMenuItemB( RTMessage )
{
	if( tfF1Pressed == TRUE )
	{
		WinHelp( HWindow, "OWLHELP.HLP", HELP_CONTEXT, HELP_MENUITEMB );
		tfF1Pressed = FALSE;
	} else {
		MessageBox( HWindow, "In Menu Item B Command", Title, MB_ICONINFORMATION );
	}
}

void TOWLHELPWnd::CMExit( RTMessage Msg )
{
	if ( tfF1Pressed == TRUE )
	{
		WinHelp( HWindow, "OWLHELP.HLP", HELP_CONTEXT, HELP_EXIT );
		tfF1Pressed = FALSE;
	} else {
		TWindow::CMExit( Msg );
	}
}

void TOWLHELPWnd::CMUHelpIndex( RTMessage )
{
	WinHelp( HWindow, "OWLHELP.HLP", HELP_INDEX, 0L );
}

void TOWLHELPWnd::CMUHelpHelp( RTMessage )
{
	WinHelp( HWindow, "WINHELP.HLP", HELP_HELPONHELP, 0L );
}

void TOWLHELPWnd::CMUHelpAbout( RTMessage )
{
        MessageBox( HWindow, "OWLHELP\nWritten using ObjectWindows\nCopyright (c) 1992 Borland",
        "About OWLHELP", MB_ICONINFORMATION );
}

int PASCAL WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLn, int nCmdShw )
{
	TOWLHELPApp App( "OWLHELPAP", hInst, hPrevInst, lpszCmdLn, nCmdShw );

	App.Run();
	return App.Status;
}

