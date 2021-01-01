// ObjectWindows - (C) Copyright 1992 by Borland International


/* This example demonstrates the use of MCI APIs in Windows 3.1 in an OWL
 *  application:
 *
 * You must have a sound board/Speaker and its device driver installed under
 * Windows 3.1 and be certain that it works -- Use the sound applet in Windows
 * Control Pannel to generate a sound.  You may copy one of the .WAV files from
 * the WINDOWS subdirectory in your system to this example's subdirectory.
 *
 * Run the .EXE.  Choose File:Open and select a .WAV file.  Choose Control:Play
 * to play the waveform.  The Control menu lets you stop/play/pause and resume.
 * The scrollbar allows random access through the waveform while it is playing.
 * This example demonstrates the use of the MCI API and a callback via
 * WM_MCINOTIFY.
 */

#include <string.h>
#include <stdio.h>
#include <owl.h>
#include <filedial.h>
#include <button.h>
#include <scrollba.h>
#include <math.h>
#include <mmsystem.h>
#include "soundply.h"

#define ID_SCROLL 	150			// Scroll bar
#define TIMER_ID 	264			// Unique timer ID.

// The waveform device opened.
WORD	wDeviceID = 0;
BOOL	flushNotify = FALSE;

class TSoundBar : public TScrollBar
{
private:
	int	waveRatio;
	LONG	waveLength;
	char	elementName[255];

	void RePosAndPlay (LONG newPos);
public:
	TSoundBar(PTWindowsObject AParent, int AnId, int X, int Y, int W,
             int H, BOOL IsHScrollBar, PTModule AModule = NULL) :
        TScrollBar(AParent, AnId, X, Y, W, H, IsHScrollBar, AModule) {};

	void ScrollSetInfo (int wRatio, LONG wLength);
   void ScrollSetName (char *eName);

	virtual void SBLineUp(RTMessage Msg) = [NF_FIRST + SB_LINEUP];
	virtual void SBLineDown(RTMessage Msg) = [NF_FIRST + SB_LINEDOWN];
	virtual void SBPageUp(RTMessage Msg) = [NF_FIRST + SB_PAGEUP];
	virtual void SBPageDown(RTMessage Msg) = [NF_FIRST + SB_PAGEDOWN];
	virtual void SBThumbPosition(RTMessage Msg) = [NF_FIRST + SB_THUMBPOSITION];
	virtual void SBTop(RTMessage Msg) = [NF_FIRST + SB_TOP];
	virtual void SBBottom(RTMessage Msg) = [NF_FIRST + SB_BOTTOM];
};


// Define a class derived from TApplication
class TSoundApp :public TApplication
{
public:
	TSoundApp(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance,
             LPSTR lpCmdLine, int nCmdShow)
		: TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow) {};
	virtual void InitMainWindow();
};

_CLASSDEF(TSoundWindow)
class TSoundWindow : public TWindow
{
private:
	char		elementName[255];
   int		running;
   int		pause;
   UINT		timeGoing;
	int		waveRatio;
   LONG		waveLength;
   TSoundBar	*tScroll;
	MCI_GENERIC_PARMS	MciGenParm;
	MCI_OPEN_PARMS		MciOpenParm;
	MCI_PLAY_PARMS		MciPlayParm;
   MCI_STATUS_PARMS	MciStatusParm;
   MCI_SET_PARMS		MciSetParm;

	void GetDeviceInfo ();
   void StopWave ();
	void StopMCI ();
public:
	TSoundWindow (PTWindowsObject Parent, LPSTR Title);
   ~TSoundWindow ();

	void UpdateSoundWindow ();

	virtual void GetWindowClass (WNDCLASS &WndClass);
   virtual void SetupWindow();
	virtual LPSTR GetClassName ();
	virtual void MciNotify (RTMessage Msg) = [WM_FIRST + MM_MCINOTIFY];
	virtual void Paint (HDC PaintDC, PAINTSTRUCT _FAR &PaintInfo);
   virtual void FileOpen (RTMessage Msg) = [CM_FIRST + SM_OPEN];
   virtual void FileExit (RTMessage Msg) = [CM_FIRST + SM_EXIT];
   virtual void PlayWave (RTMessage Msg) = [CM_FIRST + SM_PLAY];
   virtual void PauseWave (RTMessage Msg) = [CM_FIRST + SM_PAUSE];
   virtual void HelpAbout (RTMessage Msg) = [CM_FIRST + SM_ABOUT];
   virtual void IdleStuff (RTMessage Msg) = [WM_FIRST + WM_TIMER];
};

void TSoundBar::RePosAndPlay (LONG newPos)
{
MCI_PLAY_PARMS MciPlayParm;
MCI_SEEK_PARMS MciSeekParm;
MCI_SET_PARMS MciSetParm;
MCI_OPEN_PARMS MciOpenParm;
MCI_GENERIC_PARMS MciGenParm;

   // Only allow SEEK if playing.
   if (!wDeviceID)
		return;

	// Close the currently playing wave.
   flushNotify = TRUE;
	MciGenParm.dwCallback = 0L;
	mciSendCommand (wDeviceID, MCI_STOP, MCI_WAIT, (DWORD)(LPMCI_GENERIC_PARMS)&MciGenParm);
	mciSendCommand (wDeviceID, MCI_CLOSE, MCI_WAIT, (DWORD)(LPMCI_GENERIC_PARMS)&MciGenParm);

   // Open the wave again and seek to new position.
	MciOpenParm.dwCallback = 0L;
	MciOpenParm.wDeviceID = wDeviceID;
	MciOpenParm.wReserved0 = 0;
	MciOpenParm.lpstrDeviceType = NULL;
	MciOpenParm.lpstrElementName = elementName;
	MciOpenParm.lpstrAlias = NULL;

	if (mciSendCommand (wDeviceID, MCI_OPEN, MCI_WAIT| MCI_OPEN_ELEMENT,
      (DWORD)(LPMCI_OPEN_PARMS)&MciOpenParm)) {
		   MessageBox (HWindow, "Open Error", "Sound Play", MB_OK);
		   return;
      }
   wDeviceID = MciOpenParm.wDeviceID;

   // Our time scale is in SAMPLES.
	MciSetParm.dwTimeFormat = MCI_FORMAT_SAMPLES;
   if (mciSendCommand (wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT,
      (DWORD)(LPMCI_SET_PARMS)&MciSetParm)) {
	   MessageBox (HWindow, "Set Time Error", "Sound Play", MB_OK);
	   return;
   }

   // Compute new position, remember the scrollbar range has been scaled based on waveRatio.
   MciSeekParm.dwCallback = NULL;
   MciSeekParm.dwTo = ((newPos * waveRatio) > waveLength) ? waveLength : (newPos * waveRatio);
   if (mciSendCommand (wDeviceID, MCI_SEEK, MCI_TO,
      (DWORD)(LPMCI_SEEK_PARMS)&MciSeekParm)) {
	   MessageBox (HWindow, "Seek Error", "Sound Play", MB_OK);
	   return;
   }

	MciPlayParm.dwCallback = (unsigned long)HWindow;
	MciPlayParm.dwFrom = 0;
	MciPlayParm.dwTo = 0;
	if (mciSendCommand (wDeviceID, MCI_PLAY, MCI_NOTIFY,
      (DWORD) (LPMCI_PLAY_PARMS)&MciPlayParm)) {
		MessageBox (HWindow, "Play Error", "Sound Play", MB_OK);
		return;
   }
}

void TSoundBar::ScrollSetInfo (int wRatio, LONG wLength)
{
	waveRatio = wRatio;
   waveLength = wLength;
}

void TSoundBar::ScrollSetName (char *eName)
{
	strcpy (elementName, eName);
}

void TSoundBar::SBLineUp(RTMessage msg)
{
	TScrollBar::SBLineUp (msg);
	RePosAndPlay (GetPosition ());
}

void TSoundBar::SBLineDown(RTMessage msg)
{
	TScrollBar::SBLineDown (msg);
	RePosAndPlay (GetPosition ());
}

void TSoundBar::SBPageUp(RTMessage msg)
{
	TScrollBar::SBPageUp (msg);
	RePosAndPlay (GetPosition ());
}

void TSoundBar::SBPageDown(RTMessage msg)
{
	TScrollBar::SBPageDown (msg);
	RePosAndPlay (GetPosition ());
}

void TSoundBar::SBThumbPosition(RTMessage msg)
{
	TScrollBar::SBThumbPosition (msg);
	RePosAndPlay (GetPosition ());
}

void TSoundBar::SBTop(RTMessage msg)
{
	TScrollBar::SBTop (msg);
	RePosAndPlay (GetPosition ());
}

void TSoundBar::SBBottom(RTMessage msg)
{
 	TScrollBar::SBBottom (msg);
	RePosAndPlay (GetPosition ());
}


// Construct the THelloApp's MainWindow data member
void TSoundApp::InitMainWindow()
{
	MainWindow = new TSoundWindow(NULL, "OWL SoundPlay Demo ... using MCI");
}


int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	TSoundApp SoundApp ("SoundApp", hInstance, hPrevInstance, lpCmdLine,
                        nCmdShow);
	SoundApp.HAccTable = LoadAccelerators (hInstance, "ACCELERATORS_1");
	SoundApp.Run();
	return SoundApp.Status;
}

// Construct the TSoundWindow
TSoundWindow::TSoundWindow (PTWindowsObject Parent, LPSTR Title) :
    TWindow (Parent, Title)
{
   AssignMenu (ID_MENU);
	Attr.X = 50;
	Attr.Y = 100;
	Attr.W = 400;
	Attr.H = 150;

   running = 0;
   pause = 0;
   waveLength = waveRatio = 0;
   elementName[0] = '\0';

   tScroll = new TSoundBar (this, ID_SCROLL, 50, 67, 300, 0, TRUE);
}

void TSoundWindow::SetupWindow()
{
   TWindow::SetupWindow();
   tScroll->SetRange (0, 0);
}

TSoundWindow::~TSoundWindow ()
{
	StopMCI ();
}

// Paint member function
void TSoundWindow::Paint (HDC PaintDC, PAINTSTRUCT _FAR &)
{
	char buffer[100];

   // File name.
   if (lstrlen (elementName))
		TextOut (PaintDC, 5, 5, elementName, lstrlen(elementName));
   else
     	TextOut (PaintDC, 5, 5, "<No WAVEFORM file loaded>", 25);

   TextOut (PaintDC, 160, 35, "Samples:", 8);

   TextOut (PaintDC, 50, 48, "0", 1);		// Beginning value.

   // Ending number of samples.
	if (waveLength)
		sprintf (buffer, "%d", waveLength * waveRatio);
   else
      strcpy (buffer, "Unknown");
	TextOut (PaintDC, 315, 48, buffer, lstrlen (buffer));
}

// GetClassWindow member
void TSoundWindow::GetWindowClass (WNDCLASS &WndClass)
{
	TWindow::GetWindowClass(WndClass);
}

// GetClassName member
LPSTR TSoundWindow::GetClassName ()
{
	return "SoundPlay";
}

void TSoundWindow::GetDeviceInfo ()
{
WAVEOUTCAPS	wOutCaps;

	if (!waveOutGetDevCaps (wDeviceID, (LPWAVEOUTCAPS)&wOutCaps,
      sizeof (wOutCaps))) {
		MessageBox (HWindow, "GetDevCaps Error", "Sound Play", MB_OK);
		return;
	}
}

// Play the wave...
void TSoundWindow::PlayWave (RTMessage)
{
	if (!running) {
		// MCI APIs to open a device and play a .WAV file, using
      // notification to close
		MciOpenParm.dwCallback = 0L;
		MciOpenParm.wDeviceID = 0;
		MciOpenParm.wReserved0 = 0;
		MciOpenParm.lpstrDeviceType = NULL;
		MciOpenParm.lpstrElementName = elementName;
		MciOpenParm.lpstrAlias = NULL;

		if (mciSendCommand (0, MCI_OPEN, (DWORD) (MCI_WAIT | MCI_OPEN_ELEMENT),
         (DWORD)(LPMCI_OPEN_PARMS)&MciOpenParm)) {
			MessageBox (HWindow,
          "Open Error - a waveForm output device is necessary to use this demo.",
          "Sound Play", MB_OK);
			return;
		}
   	wDeviceID = MciOpenParm.wDeviceID;

      // The time format in this demo is in Samples.
      MciSetParm.dwCallback = 0L;
		MciSetParm.dwTimeFormat = MCI_FORMAT_SAMPLES;
      if (mciSendCommand (wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT,
         (DWORD)(LPMCI_SET_PARMS)&MciSetParm)) {
			MessageBox (HWindow, "SetTime Error", "Sound Play", MB_OK);
			return;
      }

		MciPlayParm.dwCallback = (unsigned long)HWindow;
		MciPlayParm.dwFrom = 0;
		MciPlayParm.dwTo = 0;
		mciSendCommand (wDeviceID, MCI_PLAY, MCI_NOTIFY,
         (DWORD) (LPMCI_PLAY_PARMS)&MciPlayParm);

      // Modify the menu to toggle PLAY to STOP, and enable PAUSE.
		HMENU hMenu = GetMenu (HWindow);
		ModifyMenu (hMenu, SM_PLAY, MF_STRING, SM_PLAY, "&Stop");
		EnableMenuItem (hMenu, SM_PAUSE, MF_ENABLED);
      // Make sure the Play/Stop toggle menu knows we're running.
		running = TRUE;

      // Start a timer to show our progress through the waveform file.
      timeGoing = SetTimer (HWindow, TIMER_ID, 500, NULL);

      // Give enough information to the scrollbar to monitor the
      // progress and issue a re-MCI_OPEN.
		tScroll->ScrollSetName (elementName);
   } else
       StopWave ();
}

void TSoundWindow::PauseWave (RTMessage)
{
	if (!pause) {
      // Pause the playing.
		MciGenParm.dwCallback = 0L;
		mciSendCommand (wDeviceID, MCI_PAUSE, MCI_WAIT,
          (DWORD)(LPMCI_GENERIC_PARMS)&MciGenParm);

      // Toggle Pause menu to Resume.
		HMENU hMenu = GetMenu (HWindow);
		ModifyMenu (hMenu, SM_PAUSE, MF_STRING, SM_PAUSE, "&Resume\tCtrl+R");
      pause = TRUE;
	} else {
      // Resume the playing.
		MciGenParm.dwCallback = 0L;
		mciSendCommand (wDeviceID, MCI_RESUME, MCI_WAIT,
         (DWORD)(LPMCI_GENERIC_PARMS)&MciGenParm);

      // Toggle Resume menu to Pause.
		HMENU hMenu = GetMenu (HWindow);
		ModifyMenu (hMenu, SM_PAUSE, MF_STRING, SM_PAUSE, "P&ause\tCtrl+P");
      pause = FALSE;
   }
}

void TSoundWindow::StopMCI ()
{
	if (timeGoing)			// Timer running, then kill it now.
		timeGoing = !KillTimer (HWindow, TIMER_ID);

   // Stop playing the waveform file and close the waveform device.
	MciGenParm.dwCallback = 0L;
	mciSendCommand (wDeviceID, MCI_STOP, MCI_WAIT,
      (DWORD)(LPMCI_GENERIC_PARMS)&MciGenParm);
	mciSendCommand (wDeviceID, MCI_CLOSE, MCI_WAIT,
      (DWORD)(LPMCI_GENERIC_PARMS)&MciGenParm);

	running = FALSE;
   wDeviceID = 0;
}

void TSoundWindow::StopWave ()
{
   if (wDeviceID) {
		StopMCI ();

      // Reset the menus to Play menu and gray the Pause menu.
	   HMENU hMenu = GetMenu (HWindow);
	   ModifyMenu (hMenu, SM_PLAY, MF_STRING, SM_PLAY, "&Play\tCtrl+P");
	   ModifyMenu (hMenu, SM_PAUSE, MF_STRING | MF_GRAYED, SM_PAUSE, "P&ause\tCtrl+A");
   }
}

void TSoundWindow::FileOpen (RTMessage)
{
char	FileName[MAXPATH];

	if ( GetApplication()->ExecDialog(new TFileDialog(this, SD_FILEOPEN, strcpy(FileName, "*.WAV"))) == IDOK )
        	if (CanClose()) {
            // Remember the waveform file to open.
	        	strcpy (elementName, FileName);

            // Turn the Play menu on.
            HMENU hMenu = GetMenu (HWindow);
            EnableMenuItem (hMenu, SM_PLAY, MF_ENABLED);

            waveLength = 0;
            waveRatio = 0;
        		tScroll->SetPosition (0);
            UpdateSoundWindow ();
         }
}

void TSoundWindow::FileExit (RTMessage)
{
	CloseWindow ();
}

// Response function MM_MCINOTIFY message when MCI_PLAY is complete.
void TSoundWindow::MciNotify (RTMessage)
{
int	loVal, hiVal;

   if (!flushNotify) { 			// Internal STOP/CLOSE, from thumb re-pos?
		StopWave ();

		// Make sure the thumb is at the end. There could be some WM_TIMER messages on the queue when
		// we kill it, thereby flushing WM_TIMER's from the message queue.
		tScroll->GetRange (loVal, hiVal);
		tScroll->SetPosition (hiVal);
	}
   flushNotify = FALSE; 			// Yes, so ignore the close.
}


void TSoundWindow::UpdateSoundWindow ()
{
RECT	r;

	// First time it's different update the scroll bar numbers.
	GetClientRect (HWindow, &r);
	InvalidateRect (HWindow, &r, TRUE);
}


void TSoundWindow::HelpAbout (RTMessage)
{
	MessageBox (HWindow, "SoundPly loads and plays waveform sound \
files (*.WAV).  The features of this demo are Play/Stop, Pause/Resume, \
and random seeks through the file via the scollbar (while the sound is \
playing).  NOTE: SoundPly will only play sounds if the machine contains \
a waveForm Multimedia device, e.g. SoundBlaster, etc.).",
   "About Sound Play", MB_OK);
}

void TSoundWindow::IdleStuff (RTMessage)
{
	if (!flushNotify) {			// Internal STOP/CLOSE, from thumb re-pos?
		MciStatusParm.dwCallback = 0L;	// No, normal close.
		MciStatusParm.dwItem = MCI_STATUS_LENGTH;
		mciSendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPMCI_STATUS_PARMS)&MciStatusParm);
		if (waveLength != MciStatusParm.dwReturn) {
                        // First time it's different update the scroll bar numbers.
                        UpdateSoundWindow ();
                	waveLength = MciStatusParm.dwReturn;
		}

        	// Compute the length and ratio and update tScroll info.
		waveRatio = (int)ceill((waveLength / 32000) + .5);
		tScroll->ScrollSetInfo (waveRatio, waveLength);
		tScroll->SetRange (0, (int)(waveLength / waveRatio));

        	// Update the current position.
 		MciStatusParm.dwCallback = 0L;
     	MciStatusParm.dwItem = MCI_STATUS_POSITION;
     	mciSendCommand (wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPMCI_STATUS_PARMS)&MciStatusParm);

     	tScroll->SetPosition ((int)(MciStatusParm.dwReturn / waveRatio));
   }

   flushNotify = FALSE; 			// Yes, ignore this close.
}

