// ObjectWindows - (C) Copyright 1992 by Borland International
//
// ttfont.cpp

#include <owl.h>
#include <string.h>
#include "ttfont.h"
#include <math.h>
#include <commdlg.h>

class TFontWindow: public TWindow
{
private:
  LOGFONT  MainFontRec,
           CornerFontRec,
           BorlandFontRec;
  COLORREF FanColor[10];
  BOOL ShadowAll,
       ShowAlignmentMarks,
       Rendering;
public:
  TFontWindow( PTWindowsObject AParent, LPSTR ATitle );
  virtual void cmAbout( TMessage& ) = [CM_FIRST + CM_ABOUT];
  virtual void cmAlignmentMarks( TMessage& ) = [CM_FIRST + CM_ALIGNMENTMARKS];
  virtual void cmFonts( TMessage& ) = [CM_FIRST + CM_FONTS];
  virtual void cmShadows( TMessage& ) = [ CM_FIRST + CM_SHADOWS ];
  virtual LPSTR GetClassName();
  virtual void GetWindowClass(WNDCLASS _FAR & AWndClass);
  virtual void Paint( HDC, PAINTSTRUCT& );
  virtual void WMGetMinMaxInfo( TMessage& ) = [WM_FIRST + WM_GETMINMAXINFO];
  virtual void WMSize( TMessage& ) = [WM_FIRST + WM_SIZE];
};


#define ROUND(x)  (floor(x + .5))
#define SQR(x)    (pow(x,2))

TFontWindow::TFontWindow( PTWindowsObject AParent, LPSTR ATitle ) :
                TWindow( AParent, ATitle)
{
  AssignMenu(100);

  MainFontRec.lfHeight = 26;
  MainFontRec.lfWidth = 10;
  MainFontRec.lfEscapement = 0;
  MainFontRec.lfOrientation = 0;
  MainFontRec.lfWeight = FW_BOLD;
  MainFontRec.lfItalic = 0;
  MainFontRec.lfUnderline = 0;
  MainFontRec.lfStrikeOut = 0;
  MainFontRec.lfCharSet = ANSI_CHARSET;
  MainFontRec.lfOutPrecision = OUT_DEFAULT_PRECIS;
  MainFontRec.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  MainFontRec.lfQuality = PROOF_QUALITY;
  MainFontRec.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;
  strcpy(MainFontRec.lfFaceName,"Times New Roman");

  CornerFontRec = MainFontRec;

  BorlandFontRec  = MainFontRec;
  BorlandFontRec.lfHeight = 60;
  BorlandFontRec.lfWidth = 0;   // choose best width for this height
  BorlandFontRec.lfWeight = 900;
  strcpy(BorlandFontRec.lfFaceName, "Arial");

  // Array of colors used to color the fan text
  FanColor[0] = RGB(255,0,0);
  FanColor[1] = RGB(128,0,0);
  FanColor[2] = RGB(255,128,0);
  FanColor[3] = RGB(80,80,0);
  FanColor[4] = RGB(80,255,0);
  FanColor[5] = RGB(0,128,0);
  FanColor[6] = RGB(0,128,255);
  FanColor[7] = RGB(0,0,255);
  FanColor[8] = RGB(128,128,128);
  FanColor[9] = RGB(255,0,0);

  ShadowAll = 0;
  ShowAlignmentMarks = 0;
  Rendering = 0;
};


LPCSTR ArcText = "TrueType";
const char* WaitText = "Windows is rendering fonts...";
const char* FanText = "Borland C++ for Windows";
const char* BorlandText = "Borland";
const int   Radius = 100;
const float Deg2Rad = M_PI / 18;

void TFontWindow::Paint( HDC DC, PAINTSTRUCT& )
{

          LOGFONT FontRec;
OUTLINETEXTMETRIC FontMetric;
              int FontHeight, x, y, j, k;
             WORD BaseWidth, DesiredExtent, FanTextLen;
            float Theta;
           LPCSTR P;
             RECT R;
             long TE;

  P = ArcText;
  FanTextLen = strlen(FanText);

  SaveDC(DC);

  if (Rendering)
  {
    // display a message that Windows is rendering fonts, please wait...
    SetWindowText(HWindow, WaitText);
  };

  FontRec = CornerFontRec;
  SetBkMode(DC, TRANSPARENT);
  SetTextColor(DC, RGB(128,128,128));
  FontRec.lfHeight = FontRec.lfHeight * 2;
  FontRec.lfWidth = floor(FontRec.lfWidth * 2.1);
  SelectObject(DC, CreateFontIndirect(&FontRec));
  TextOut(DC, 18, 5, "T", 1);
  SetTextColor(DC, RGB(0,0,0));
  TextOut(DC, 32, 13,"T", 1);

  GetClientRect(HWindow, &R);

  FontRec = MainFontRec;
  DeleteObject(SelectObject(DC, CreateFontIndirect(&FontRec)));
  GetOutlineTextMetrics(DC, sizeof(FontMetric), &FontMetric);
  FontHeight = FontMetric.otmTextMetrics.tmHeight;
  SetViewportOrg(DC, FontHeight+2, 0);
  R.right -= FontHeight+2;
  BaseWidth = LOWORD(GetTextExtent(DC, FanText, FanTextLen));

  SelectObject(DC, GetStockObject(NULL_BRUSH));
  if (ShowAlignmentMarks)
  { Ellipse(DC, -R.right, -R.bottom, R.right, R.bottom);};
  Ellipse(DC, -(Radius-5), -(Radius-5), (Radius-5), Radius-5);
  Ellipse(DC, -(Radius-10), -(Radius-10), (Radius-10), Radius-10);

  SetTextColor(DC, FanColor[0]);
  for (int d = 27; d <= 36; d++)
  {
    x = ROUND(Radius * cos(d * Deg2Rad));
    y = ROUND(Radius * sin(-d * Deg2Rad)); // -d because y axis is inverted

    Theta = -d * Deg2Rad;
    if (x)
      { Theta = atan((R.right / (R.bottom * 1.0)) * (y / (x * 1.0))); };
    j = ROUND(R.right * cos(Theta));
    k = ROUND(R.bottom * sin(Theta));

    if (ShowAlignmentMarks)
    {
      MoveTo(DC, x,y);
      LineTo(DC, j,k);
    };

    DesiredExtent = ROUND(sqrt(SQR(x*1.0 - j) + SQR(y*1.0 - k))) - 5;
    FontRec = MainFontRec;
    FontRec.lfEscapement = d * 100;
    FontRec.lfWidth = floor((FontMetric.otmTextMetrics.tmAveCharWidth) * (DesiredExtent / (BaseWidth * 1.0)));
    DeleteObject(SelectObject(DC, CreateFontIndirect(&FontRec)));
    TE = GetTextExtent(DC, FanText, FanTextLen);

    for ( ;(LOWORD(TE) > DesiredExtent) && (FontRec.lfWidth); FontRec.lfWidth-- )
    { // Shave off some character width until the string fits
      DeleteObject(SelectObject(DC, CreateFontIndirect(&FontRec)));
      TE = GetTextExtent(DC, FanText, FanTextLen);
    };

    // Expand the string if necessary to make it fit the desired extent
    if (LOWORD(TE) < DesiredExtent)
      { SetTextJustification(DC,DesiredExtent - LOWORD(TE), 3); };
    if (ShadowAll)
    {
      SetTextColor(DC, RGB(0,0,0));
      TextOut(DC, x+2, y+1, FanText, FanTextLen);
    };
    SetTextColor(DC, FanColor[d - 27]);
    TextOut(DC, x, y, FanText, FanTextLen);
    SetTextJustification(DC,0,0);  // clear justifier's internal error accumulator

    if (P[0])
    {
      FontRec = CornerFontRec;
      FontRec.lfEscapement = (d+10) * 100;
      FontRec.lfWidth = 0;
      DeleteObject(SelectObject(DC, CreateFontIndirect(&FontRec)));
      SetTextColor(DC, 0);
      x = floor((Radius - FontHeight - 5) * cos(d * Deg2Rad));
      y = floor((Radius - FontHeight - 5) * sin(-d * Deg2Rad));
      TextOut(DC, x, y, P, 1);
      P++;
    };
  };

  DeleteObject(SelectObject(DC, CreateFontIndirect(&BorlandFontRec)));
  TE = GetTextExtent(DC, BorlandText, strlen(BorlandText));
  SetTextColor(DC, RGB(0,0,0));
  TextOut(DC, R.right - LOWORD(TE), R.bottom - HIWORD(TE), BorlandText, strlen(BorlandText));
  SetTextColor(DC, RGB(255,0,0));
  TextOut(DC, R.right - LOWORD(TE) - 5, R.bottom - HIWORD(TE), BorlandText, strlen(BorlandText));

  if (Rendering)
  {
    // restore the window caption to the proper title string
    SetWindowText(HWindow, Title);
    // clear the rendering flag.  It will be set again when the window is resized (WMSIZE)
    Rendering = 0;
  };

  DeleteObject(SelectObject(DC, GetStockObject(SYSTEM_FONT)));
  RestoreDC(DC, -1);
};


void TFontWindow::cmShadows( TMessage& )
{
  ShadowAll = !ShadowAll;
  if (ShadowAll)
  { CheckMenuItem(GetMenu(HWindow), CM_SHADOWS, MF_BYCOMMAND | MF_CHECKED); }
  else
  { CheckMenuItem(GetMenu(HWindow), CM_SHADOWS, MF_BYCOMMAND | MF_UNCHECKED); };

  // Erase if going Shadow -> no Shadow
  InvalidateRect(HWindow, NULL, !ShadowAll);
};

void TFontWindow::cmAlignmentMarks( TMessage& )
{
  ShowAlignmentMarks = !ShowAlignmentMarks;
  if (ShowAlignmentMarks)
  { CheckMenuItem(GetMenu(HWindow), CM_ALIGNMENTMARKS, MF_BYCOMMAND | MF_CHECKED); }
  else
  { CheckMenuItem(GetMenu(HWindow), CM_ALIGNMENTMARKS, MF_BYCOMMAND | MF_UNCHECKED); };

  // Erase if going Marks -> no Marks
  InvalidateRect(HWindow, NULL, !ShowAlignmentMarks);
};

void TFontWindow::WMGetMinMaxInfo( TMessage& Msg )
{
  // Limit the minimum size of the window to 300x300, so the fonts don't
  //  get too small }
  ((POINT far *)Msg.LParam)[3].x = 300;
  ((POINT far *)Msg.LParam)[3].y = 300;
};


void TFontWindow::cmAbout( TMessage& )
{
  GetApplication()->ExecDialog(new TDialog(this, "About"));
};

void TFontWindow::cmFonts( TMessage& )
{
  CHOOSEFONT CF;
  LOGFONT FontRec = MainFontRec;

  CF.lStructSize = sizeof(CF);
  CF.hwndOwner = HWindow;
  CF.Flags = CF_ANSIONLY | CF_TTONLY | CF_SCREENFONTS
	     | CF_INITTOLOGFONTSTRUCT | CF_ENABLETEMPLATE;
  CF.nFontType = SCREEN_FONTTYPE;
  CF.lpLogFont = &FontRec;
  CF.nSizeMin = 20;
  CF.nSizeMax = 20;
  CF.lpTemplateName = "FontDlg";
  CF.hInstance = GetApplication()->hInstance;

  if (ChooseFont(&CF))
  {
    // Only get the font name, weight, and italics - we don't care about size }
    strcpy(MainFontRec.lfFaceName, FontRec.lfFaceName);
    MainFontRec.lfWeight = FontRec.lfWeight;
    MainFontRec.lfItalic = FontRec.lfItalic;
    Rendering = 1;
    InvalidateRect(HWindow, NULL, TRUE);
  };
};


LPSTR TFontWindow::GetClassName()
{
  return "OWLTrueTypeFontDemoWindow";
};


void TFontWindow::GetWindowClass(WNDCLASS _FAR & AWndClass)
{
 TWindow::GetWindowClass(AWndClass);
 AWndClass.hIcon = LoadIcon(GetApplication()->hInstance, MAKEINTRESOURCE(1));
};

void TFontWindow::WMSize( TMessage& Msg )
{
 TWindow::WMSize(Msg);
 if ((Msg.WParam == SIZENORMAL) || (Msg.WParam == SIZEFULLSCREEN))
 {  Rendering = 1; };
};





// Define a class derived from TApplication
class THelloApp :public TApplication
{
public:
  THelloApp(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
    : TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow) {};
  virtual void InitMainWindow();
};

// Construct the THelloApp's MainWindow data member
void THelloApp::InitMainWindow()
{
  MainWindow = new TFontWindow(NULL, "TrueType Font Lab");
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
  LPSTR lpCmdLine, int nCmdShow)
{
  THelloApp HelloApp ("HelloApp", hInstance, hPrevInstance,
    lpCmdLine, nCmdShow);
  HelloApp.Run();
  return HelloApp.Status;
}
