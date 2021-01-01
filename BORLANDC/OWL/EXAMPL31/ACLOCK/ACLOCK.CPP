// ObjectWindows - (C) Copyright 1992 by Borland International

#include <alloc.h>
#include <owl.h>
#include <window.h>
#include <dos.h>
#include <stdlib.h>
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include <bios.h>>
#include "aclock.h"
#include "bitmap.h"

const float pi2 = 2*3.141592;

// Implementation of CompatibleDC and Bitmap classes.

class CompatibleDC
{
    private:
	HDC hDCMem;
    public:
	// Create a memory device context, specify a selected object,
	// and set the DC's mapping mode.
	CompatibleDC( HDC hDC )
	{
	    hDCMem = CreateCompatibleDC( hDC );
	    SetMapMode( hDCMem, GetMapMode( hDC ) );
	}
	~CompatibleDC( void ) { DeleteDC( hDCMem ); };
	HDC Get_hDCMem( void ) { return hDCMem; }
};

// draw a bitmap
void FAR Bitmap::Display( HDC hDC, short xStart, short yStart)
{
    POINT ptSize, ptOrigin;

    CompatibleDC MemoryDC( hDC );
    HDC hDCMem = MemoryDC.Get_hDCMem();
    SelectObject( hDCMem, hBitmap );

    ptSize = GetSize( hDC );
    ptOrigin.x = 0;
    ptOrigin.y = 0;
    DPtoLP( hDCMem, &ptOrigin, 1 );

    BitBlt( hDC, xStart, yStart, ptSize.x, ptSize.y,
	    hDCMem, ptOrigin.x, ptOrigin.y, SRCCOPY );
}


//Animated class animates a set of bitmaps
class Animated
{
private:
  int numBmps;     //number of bitmaps
  Bitmap **pBmps; //handles to bitmaps
public:
  int delayTicks;   //amount to delay between frames
		    //18.2 ticks/sec
  int delayEnd;     //amount to delay after sequence
  virtual void Display(HDC hDC, int x, int y);
  Animated(HINSTANCE hInst, int numB, char *name);
  virtual ~Animated();
};


//Create an animated sequence
Animated::Animated(HINSTANCE hInst, int numB, char *name)
{
  int j;
  char resName[40];

  //create array of numB Bitmap*'s
  pBmps = new (Bitmap (*[numB]));

  //load in bitmap resources
  numBmps = numB;
  for (j=0; j<numB; j++) {
    sprintf(resName, "%s%d", name, j+1);
    pBmps[j] = new Bitmap(hInst, resName );
  }
}

Animated::~Animated()
{
   for (int j=0; j<numBmps ;j++ )
      delete pBmps[j];
}

//Draw the series of bitmaps
void Animated::Display(HDC hDC, int x, int y)
{
  int j;
  long endTime;

  //draw each bitmap and then delay
  for (j=0; j<numBmps; j++) {
    pBmps[j]->Display(hDC, x, y);
    //delay for delayTicks clock ticks (18.2/sec)
    endTime = biostime(0, 0L) + delayTicks;
    while (biostime(0, 0L) < endTime);

  }
  //delay for ending amount
  endTime = biostime(0, 0L) + delayEnd;
  while (biostime(0, 0L) < endTime)
    Yield();              //be nice to Windows
}


// Application is based on TApplication
_CLASSDEF(TAClockApp)
class TAClockApp : public TApplication
{
public:
  TAClockApp(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
    : TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow) {};
  virtual void InitMainWindow();

  friend class TAClockWindow;
};


//Clock window is based on TWindow
class TAClockWindow : public TWindow
{
private:
  Bitmap *phbmFace;        //clock face bitmap
  POINT Center;            //center of clock
  int	Radius;            //clock radius
  struct time  LastTime;   //last time drawn
  HPEN hpHour, hpMinute;   //pens for clock hands
  Animated *panCuckoo;     //cuckoo sequence

public:

  TAClockWindow(PTWindowsObject AParent, LPSTR ATitle);
  virtual ~TAClockWindow();
  virtual void SetupWindow();
  virtual void Paint(HDC PaintDC, PAINTSTRUCT _FAR & PaintInfo);
  virtual void WMTimer(RTMessage Msg) = [WM_FIRST + WM_TIMER];
  virtual void CMAbout(RTMessage  Msg) =
    [CM_FIRST + CM_ABOUT];
  virtual void CMEffectChime(RTMessage Msg) = [CM_FIRST + CM_EFFECTCHIME];
  virtual void CMEffectCuckoo(RTMessage Msg) = [CM_FIRST + CM_EFFECTCUCKOO];
  void PaintHands(HDC hDC, struct time theTime);
  void Chime();
  void Cuckoo();
};


TAClockWindow::~TAClockWindow()
{
  //get rid of timer
  KillTimer(HWindow ,1);
  //delete brushes
  DeleteObject(hpMinute);
  DeleteObject(hpHour);
  // delete bitmaps
  delete phbmFace;
  // deleted Animated object
  delete panCuckoo;
}


//load sequences and set size in the constructor
TAClockWindow::TAClockWindow(PTWindowsObject AParent, LPSTR ATitle) :
  TWindow(AParent, ATitle)
{
  AssignMenu("TOOL_MENU");
  PTAClockApp Application = (PTAClockApp) GetApplication();
  //load face bitmap and cuckoo sequence
  phbmFace = new Bitmap(Application->hInstance, "FACE_BMP");
  panCuckoo = new Animated(Application->hInstance, 8, "CUCKOO");
  //set frame delay time
  panCuckoo->delayTicks = 5;
  panCuckoo->delayEnd = 48;
  //Set the window size
  HDC TheDC = GetDC(HWindow);
  POINT temp = phbmFace->GetSize(TheDC);
  //set to size of bitmap plus fudge factor
  Attr.H = temp.y + 40;
  Attr.W = temp.x + 2;
  Attr.Style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
  ReleaseDC(HWindow, TheDC);
  Center.x = temp.x / 2;
  Center.y = temp.y / 2;
  Radius = 3*(Center.x < Center.y ? Center.x : Center.y) / 4;
}

//do any hWindow required tasks here
void TAClockWindow::SetupWindow()
{
  //set the timer - get called every minute
  if (!SetTimer(HWindow, 1, 60000U, NULL))
	MessageBox(HWindow, "Too Many Timers", "Cuckoo Clock", MB_ICONEXCLAMATION | MB_OK);
  //create the pens for the clock hands
  hpHour = CreatePen(PS_SOLID, 4, 0x00FFFF);
  hpMinute = CreatePen(PS_SOLID, 4, 0xFF00FFUL);
}


//Paint the clock background
void TAClockWindow::Paint(HDC PaintDC, PAINTSTRUCT _FAR &)
{
  //paint the face
  phbmFace->Display(PaintDC, 0, 0);
  //paint the first time
  gettime(&LastTime);
  PaintHands(PaintDC, LastTime);
}


void TAClockWindow::PaintHands(HDC hDC, struct time theTime)
{
  float hourAngle, minuteAngle;
  POINT hourPt, minutePt;

  //compute the location of the hands
  minuteAngle = theTime.ti_min * pi2 / 60;
  hourAngle = (theTime.ti_hour % 12) * pi2 / 12 + minuteAngle/12;
  hourPt.x = (Radius/2)*sin(hourAngle) + Center.x;
  hourPt.y = (-Radius/2)*cos(hourAngle) + Center.y;
  minutePt.x = Radius*sin(minuteAngle) + Center.x;
  minutePt.y = -Radius*cos(minuteAngle) + Center.y;

  //now draw them.  Note use of XOR
  SetROP2(hDC, R2_XORPEN);
  SelectObject(hDC, hpHour);
  MoveTo(hDC, Center.x, Center.y);
  LineTo(hDC, hourPt.x, hourPt.y);
  SelectObject(hDC, hpMinute);
  MoveTo(hDC, Center.x, Center.y);
  LineTo(hDC, minutePt.x, minutePt.y);
}

void TAClockWindow::Chime(void)
{
  //stop any currently playing sound
  sndPlaySound(NULL,0);
  //play a sound, based on the WIN.INI entry
  //to use a disk file, specify the disk file instead of the
  //WIN.INI entry, such as "tada.wav"
  sndPlaySound("SystemAsterisk", SND_ASYNC);
}


void TAClockWindow::Cuckoo(void)
{
  //stop any currently playing sound
  sndPlaySound(NULL,0);
  //play a sound, based on the WIN.INI entry
  sndPlaySound("SystemExclamation", SND_ASYNC);
  // do cute graphics
  HDC hDC = GetDC(HWindow);
  panCuckoo->Display(hDC, 55, 125);
  //force a repaint of the whole thing
  InvalidateRect(HWindow, NULL, 1);
  ReleaseDC(HWindow, hDC);
}

//Called each minute - paints hands
void TAClockWindow::WMTimer(RTMessage)
{
  HDC hDC = GetDC(HWindow);
  PaintHands(hDC, LastTime);
  gettime(&LastTime);
  PaintHands(hDC, LastTime);
  ReleaseDC(HWindow, hDC);

  //Every hour play a chime
  if (LastTime.ti_min == 0)
     Chime();

  //if midnight, time for the cuckoo
  if ((LastTime.ti_hour == 0) && (LastTime.ti_min == 0))
     Cuckoo();
}

//About dialog box
void TAClockWindow::CMAbout(RTMessage)
{
  GetApplication()->ExecDialog(new TDialog(this, "ABOUT"));
}

// Menu item Effect:Chime
void TAClockWindow::CMEffectChime(RTMessage)
{
   Chime();
}

// Menu item Effect:Cuckoo
void TAClockWindow::CMEffectCuckoo(RTMessage)
{
   Cuckoo();
}


void TAClockApp::InitMainWindow()
{
  MainWindow = new TAClockWindow(NULL, "Cuckoo Clock");
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpCmdLine, int nCmdShow)
{
  HINSTANCE hBorLibrary;

  //Load Borland custom controls
  hBorLibrary = LoadLibrary("bwcc.dll");
  if((int) hBorLibrary <= 32)
     MessageBox(NULL, "Unable to load Borland Controls", "System Error", MB_OK | MB_ICONHAND);

  TAClockApp AClockApp("Cuckoo Clock", hInstance, hPrevInstance,
    lpCmdLine, nCmdShow);
  AClockApp.Run();

  //Free Borland custom controls
  if((int) hBorLibrary > 32)
     FreeLibrary(hBorLibrary);
  return AClockApp.Status;
}
