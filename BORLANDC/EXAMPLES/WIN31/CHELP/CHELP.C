// Windows 3.1 Demo Program
// Copyright (c) 1992 by Borland International

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

#define STRICT

#include <windows.h>

#include "chelpm.h"
#include "chelpr.h"

long FAR PASCAL _export WndProc( HWND, WORD , WORD, LONG );

#pragma argsused
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpszCmd, int nCmdShow)
{
	 static char szAppName[] = "CHelp" ;
	 HWND        hwnd ;
	 MSG         msg ;
	 WNDCLASS    wndclass ;
	 HANDLE 	 hAccel;

	 if (!hPrevInstance)
	 {
		  wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
		  wndclass.lpfnWndProc   = (WNDPROC)WndProc ;
		  wndclass.cbClsExtra    = 0 ;
		  wndclass.cbWndExtra    = 0 ;
		  wndclass.hInstance     = hInstance ;
		  wndclass.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
		  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
		  wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		  wndclass.lpszMenuName  = MAKEINTRESOURCE( CHELPAPMENU );
		  wndclass.lpszClassName = szAppName ;

		  RegisterClass (&wndclass) ;
	}

	hwnd = CreateWindow (szAppName,
		"Windows Help Example",
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

	 hAccel = LoadAccelerators( hInstance,
			MAKEINTRESOURCE( CHELPAPACCEL ) );

	 while (GetMessage (&msg, NULL, 0, 0))
	 {
		if ( !TranslateAccelerator( hwnd, hAccel, &msg ))
		{
			 TranslateMessage (&msg) ;
			 DispatchMessage (&msg) ;
		}
	 }
	 return msg.wParam ;
}

LPSTR lpszClientAreaText = "To Get help, press F1 while a menu item is \
highlighted.  WinHelp will be called to show help on that topic.  If help \
is not shown, it is because the mouse button is pressed.  Release the \
mouse button to see help.";

long FAR PASCAL _export WndProc (HWND hwnd, WORD message, WORD wParam, LONG lParam)
{
	 static BOOL tfF1Pressed = FALSE;

	 switch (message)
	 {
		  case WM_PAINT:
		  {
			  HDC hdc;
			  PAINTSTRUCT ps;
			  RECT rectClient;
			  int nWidth, nHeight;

			  hdc = BeginPaint( hwnd, &ps );
			  GetClientRect( hwnd , &rectClient );

			  nWidth = rectClient.right;		// rectClient.left is 0
			  nHeight = rectClient.bottom;	// rectClient.top is 0

			  rectClient.left += (nWidth / 4);
			  rectClient.top  += (nHeight / 4);
			  rectClient.right -= (nWidth / 4);

			  SetBkMode( hdc, TRANSPARENT );
			  DrawText( hdc, lpszClientAreaText, lstrlen( lpszClientAreaText ),
				  &rectClient, DT_CENTER | DT_VCENTER | DT_WORDBREAK );
			  EndPaint( hwnd, &ps );
		  }
		  break;

		  case WM_ENTERIDLE:
		  {
			  if( (wParam == MSGF_MENU ) &&
				  ((GetKeyState( VK_F1 ) & 0x8000) != 0) )
					// The key is down if the high order bit is set.
			  {
				  tfF1Pressed = TRUE;
				  PostMessage( hwnd, WM_KEYDOWN, VK_RETURN, 0L );
			  }
		  }
		  break;
		  case WM_COMMAND:
			switch ( wParam )
			{
				case CM_U_MENUITEMA:
				{
					if( tfF1Pressed == TRUE )
					{
						WinHelp( hwnd, "CHELP.HLP", HELP_CONTEXT, HELP_MENUITEMA );
						tfF1Pressed = FALSE;
					} else {
						MessageBox( hwnd, "In Menu Item A command", "C Help Example", MB_ICONINFORMATION );
					}
				}
				break;
			case  CM_U_MENUITEMB :
			{
				if( tfF1Pressed == TRUE )
				{
					WinHelp( hwnd, "CHELP.HLP", HELP_CONTEXT, HELP_MENUITEMB );
					tfF1Pressed = FALSE;
				} else {
					MessageBox( hwnd, "In Menu Item B Command", "C help Example", MB_ICONINFORMATION );
				}
			}
			break;
			case CM_U_HELPINDEX:
				WinHelp( hwnd, "CHELP.HLP", HELP_INDEX, 0L );
			break;
			case CM_U_HELPHELP:
				WinHelp( hwnd, "CHELP.HLP", HELP_HELPONHELP, 0L );
			break;
			case CM_U_HELPABOUT:
               MessageBox( hwnd,"CHELP\n\nCopyright (c) 1992 Borland International",
				"About CHELP", MB_ICONINFORMATION );
			break;
			case CM_EXIT:
			if ( tfF1Pressed == TRUE )
			{
				WinHelp( hwnd, "CHELP.HLP", HELP_CONTEXT, HELP_EXIT );
				tfF1Pressed = FALSE;
			} else {
				DestroyWindow( hwnd );
			}
			break;
		  }
		  break;

		  case WM_DESTROY:
			   PostQuitMessage (0) ;
			   return 0 ;
	 }
	 return DefWindowProc (hwnd, message, wParam, lParam) ;
}

