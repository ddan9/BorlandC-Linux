// Borland C++ - (C) Copyright 1992 by Borland International

//*******************************************************************
//
// program - sounder.c
// purpose - a C windows program to demonstrate sound APIs in Win 3.1
//
//*******************************************************************

#define STRICT

#include <windows.h>
#include <mmsystem.h>

#include "sounder.h"

// data initialized by first instance
typedef struct tagSETUPDATA
	{
		char   szAppName[20]; // name of application
	} SETUPDATA;

SETUPDATA SetUpData;

// Data that can be referenced throughout the
// program but not passed to other instances

HANDLE    hInst;                              // hInstance of application
HWND      hWndMain;                           // hWnd of main window
char      szAppName[10];                      // name of application

// function prototypes

int      PASCAL        WinMain(HINSTANCE,HINSTANCE,LPSTR,int);
void                   InitSound(HINSTANCE,HINSTANCE,LPSTR,int);
void                   InitSoundFirst(HINSTANCE);
void                   InitSoundAdded(HINSTANCE);
void                   InitSoundEvery(HINSTANCE,int);
LRESULT CALLBACK       SounderWndProc(HWND,UINT,WPARAM,LPARAM);
void                   SoundPaint(HWND);

//*******************************************************************
// WinMain - sounder main
//
// paramaters:
//             hInstance     - The instance of this instance of this
//                             application.
//             hPrevInstance - The instance of the previous instance
//                             of this application. This will be 0
//                             if this is the first instance.
//             lpszCmdLine   - A long pointer to the command line that
//                             started this application.
//             cmdShow       - Indicates how the window is to be shown
//                             initially. ie. SW_SHOWNORMAL, SW_HIDE,
//                             SW_MIMIMIZE.
//
// returns:
//             wParam from last message.
//
//*******************************************************************
int PASCAL WinMain(
HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR lpszCmdLine,
int cmdShow)
{
		MSG   msg;

		// Go init this application.
		InitSound(hInstance, hPrevInstance, lpszCmdLine, cmdShow);

		// Get and dispatch messages for this applicaton.
		while (GetMessage(&msg, NULL, 0, 0))
		{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
		}

		return(msg.wParam);
}

//*******************************************************************
// InitSound - init the Sound application
//
// paramaters:
//             hInstance     - The instance of this instance of this
//                             application.
//             hPrevInstance - The instance of the previous instance
//                             of this application. This will be 0
//                             if this is the first instance.
//             lpszCmdLine   - A long pointer to the command line that
//                             started this application.
//             cmdShow       - Indicates how the window is to be shown
//                             initially. ie. SW_SHOWNORMAL, SW_HIDE,
//                             SW_MIMIMIZE.
//
//*******************************************************************
#pragma argsused
void InitSound(
HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR lpszCmdLine,
int cmdShow)
{
		if (! hPrevInstance)           // if no previous instance, this is first
				InitSoundFirst(hInstance);
		else
				InitSoundAdded(hPrevInstance);   // this is not first instance

		InitSoundEvery(hInstance, cmdShow);  // initialization for all instances
}

//*******************************************************************
// InitSoundFirst - done only for first instance of Sounder
//
// paramaters:
//             hInstance     - The instance of this instance of this
//                             application.
//
//*******************************************************************
void InitSoundFirst(
HINSTANCE hInstance)
{
		WNDCLASS wcSoundClass;

		// Get string from resource with application name.
		LoadString(hInstance, IDS_NAME, (LPSTR) SetUpData.szAppName, 10);

		// Define the window class for this application.
		wcSoundClass.lpszClassName = SetUpData.szAppName;
		wcSoundClass.hInstance     = hInstance;
		wcSoundClass.lpfnWndProc   = SounderWndProc;
		wcSoundClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wcSoundClass.hIcon         = LoadIcon(hInstance, SetUpData.szAppName);
		wcSoundClass.lpszMenuName  = NULL;
		wcSoundClass.hbrBackground = GetStockObject(WHITE_BRUSH);
		wcSoundClass.style         = CS_HREDRAW | CS_VREDRAW;
		wcSoundClass.cbClsExtra    = 0;
		wcSoundClass.cbWndExtra    = 0;

		// Register the class
		RegisterClass(&wcSoundClass);
}

//*******************************************************************
// InitSoundAdded - done only for added instances of Sound
//
// paramaters:
//             hPrevInstance - The instance of the previous instance
//                             of this application.
//
//*******************************************************************
void InitSoundAdded(
HINSTANCE hPrevInstance)
{
		// get the results of the initialization of first instance
		GetInstanceData(hPrevInstance, (BYTE*)&SetUpData, sizeof(SETUPDATA));
}

//*******************************************************************
// InitSoundEvery - done for every instance of Sound
//
// paramaters:
//             hInstance     - The instance of this instance of this
//                             application.
//             cmdShow       - Indicates how the window is to be shown
//                             initially. ie. SW_SHOWNORMAL, SW_HIDE,
//                             SW_MIMIMIZE.
//
//*******************************************************************
void InitSoundEvery(
HINSTANCE hInstance,
int cmdShow)
{
		hInst = hInstance;       // save for use by window procs

		// Create applications main window.
		hWndMain = CreateWindow(
									SetUpData.szAppName,     // window class name
									SetUpData.szAppName,     // window title
									WS_OVERLAPPEDWINDOW,     // type of window
									100,           			 // x  window location
									100,                     // y
									450,           			 // cx and size
									250,                     // cy
									NULL,                    // no parent for this window
									NULL,                    // use the class menu
									hInstance,               // who created this window
									NULL                     // no parms to pass on
									);

		// Update display of main window.
		ShowWindow(hWndMain, cmdShow);
		UpdateWindow(hWndMain);
}


//*******************************************************************
// SoundWndProc - handles messages for this application
//
// paramaters:
//             hWnd          - The window handle for this message
//             message       - The message number
//             wParam        - The WPARAM parmater for this message
//             lParam        - The LPARAM parmater for this message
//
// returns:
//             depends on message.
//
//*******************************************************************
LRESULT CALLBACK SounderWndProc(
HWND hWnd,
UINT message,
WPARAM wParam,
LPARAM lParam)
{
		HDC hdc;
		RECT rct;
		DWORD res;
		static char Buffer[100], Buffer1[100];
		static MCI_GENERIC_PARMS MciGenParm;
		static MCI_OPEN_PARMS MciOpenParm;
		static MCI_PLAY_PARMS MciPlayParm;
		WORD wDeviceID;

		switch (message)
		{
				case WM_CREATE:
						CreateWindow ("button", "&Clear",
													WS_CHILD| WS_VISIBLE| BS_PUSHBUTTON,
													20, 40, 120, 30,
													hWnd, (HMENU)IDBT_CLEAR, hInst, NULL);

						CreateWindow ("button", "&sndSoundPlay",
													WS_CHILD| WS_VISIBLE| BS_PUSHBUTTON,
													20, 80, 120, 30,
													hWnd, (HMENU)IDBT_SNDPLAY, hInst, NULL);

						CreateWindow ("button", "Mci &Command",
													WS_CHILD| WS_VISIBLE| BS_PUSHBUTTON,
													20, 120, 120, 30,
													hWnd, (HMENU)IDBT_MCICMD, hInst, NULL);

						CreateWindow ("button", "Mci &String",
													WS_CHILD| WS_VISIBLE| BS_PUSHBUTTON,
													20, 160, 120, 30,
													hWnd, (HMENU)IDBT_MCISTR, hInst, NULL);

						return(DefWindowProc(hWnd, message, wParam, lParam));

				case WM_COMMAND:
						switch (wParam)
						{
								case IDBT_CLEAR:
										hdc = GetDC (hWnd);
										GetClientRect (hWnd, &rct);
										BitBlt (hdc, 150, 40, rct.right, rct.bottom, NULL, 0, 0, WHITENESS);
										TextOut (hdc, 20, 5, "Is your sound board and setup installed?", 40);
										ReleaseDC (hWnd, hdc);
										break;

								case IDBT_SNDPLAY:
										hdc = GetDC (hWnd);
										TextOut (hdc, 150, 80, "API: sndPlaySound()", 19);
										ReleaseDC (hWnd, hdc);
										sndPlaySound ("sounder.wav", SND_SYNC);
										break;

								case IDBT_MCISTR:
										hdc = GetDC (hWnd);
										wsprintf (Buffer1, "API: mciSendString() open/play/stop/close");
										TextOut (hdc, 150, 160, Buffer1, lstrlen(Buffer1));
										res = mciSendString ((LPSTR)"open sounder.wav alias sounder", Buffer, 80, NULL);
										if (!res)
											MessageBox (hWnd, "Play", "Sounder", MB_OK);

										res = mciSendString ((LPSTR)"play sounder", Buffer, 80, NULL);
										if (!res)
											MessageBox (hWnd, "Stop", "Sounder", MB_OK);

										res = mciSendString ((LPSTR)"stop sounder", NULL, 0, NULL);
										if (!res)
											MessageBox (hWnd, "Close", "Sounder", MB_OK);

										res = mciSendString ((LPSTR)"close sounder", NULL, 0, NULL);
										ReleaseDC (hWnd, hdc);
										break;

								case IDBT_MCICMD:
										hdc = GetDC (hWnd);
										wsprintf (Buffer1, "API: mciSendCommand()");
										TextOut (hdc, 150, 120, Buffer1, lstrlen(Buffer1));
										MciOpenParm.dwCallback = 0L;
										MciOpenParm.wDeviceID = 0;
										MciOpenParm.wReserved0 = 0;
										MciOpenParm.lpstrDeviceType = NULL;
										MciOpenParm.lpstrElementName = (LPSTR)"SOUNDER.WAV";
										MciOpenParm.lpstrAlias = NULL;
										res = mciSendCommand (0, MCI_OPEN, MCI_WAIT| MCI_OPEN_ELEMENT,
																					(DWORD)(LPMCI_OPEN_PARMS)&MciOpenParm);
										if (!res)
											MessageBox (hWnd, "Play", "Sounder", MB_OK);

										MciPlayParm.dwCallback = (unsigned long)hWnd;
										MciPlayParm.dwFrom = 0;
										MciPlayParm.dwTo = 0;
										wDeviceID = MciOpenParm.wDeviceID;
										res = mciSendCommand (wDeviceID, MCI_PLAY, MCI_NOTIFY,
																					(DWORD) (LPMCI_PLAY_PARMS)&MciPlayParm);
										if (!res)
											MessageBox (hWnd, "Stop", "Sounder", MB_OK);

										MciGenParm.dwCallback = 0L;
										res = mciSendCommand (wDeviceID, MCI_STOP, MCI_WAIT,
																					(DWORD)(LPMCI_GENERIC_PARMS)&MciGenParm);
										if (!res)
											MessageBox (hWnd, "Close", "Sounder", MB_OK);

										res = mciSendCommand (wDeviceID, MCI_CLOSE, MCI_WAIT,
																					(DWORD)(LPMCI_GENERIC_PARMS)&MciGenParm);
										ReleaseDC (hWnd, hdc);
										break;

								default:
										break;
						}
					break;

				case WM_DESTROY:
						PostQuitMessage(0);   // this is the end...
						break;

				case WM_CLOSE:
						// Tell windows to destroy our window.
						DestroyWindow(hWnd);
						break;

				default:
					// Let windows handle all messages we choose to ignore.
					return(DefWindowProc(hWnd, message, wParam, lParam));
		}
		return(0L);
}
