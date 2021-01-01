 // Borland C++ - (C) Copyright 1991, 1992 by Borland International

/*******************************************************************

    program - TrueType.c
    purpose - a Windows program to demo True Type fonts.


    This program shows off two features of Windows 3.1: True Type
fonts, and common dialogs.

    True Type fonts are scalable fonts supplied by the system.  This 
program demonstrates resizing and rotation of True Type characters
within a window, and allows the user to select the True Type font to 
be used. 

    The dialog used in selecting the font is a Windows 3.1 "common 
dialog."  By including commdlg.h, a series of common dialogs becomes 
available to the program.  To invoke the font dialog, a CHOOSEFONTS 
structure is filled out, and ChooseFont() is called.  This system 
routine handles all the dialog processing needed to select a font via 
a dialog box.

    In contrast, the About Box dialog herein is programmed in the 
normal manner.  A dialog process is provided, and a thunk for it is 
created explicitly; Windows provides no automated handling for this 
program-specific dialog.

    Change font attributes through the menu selections.  Resize the
window to see the text resized; the text "fan" will always reach the
bottom and the right side of the window.

*******************************************************************/ 

#define STRICT

#include <windows.h>
#include <string.h>
#include <math.h>
#include <commdlg.h>

#include "TrueType.h"


// data initialized by first instance
typedef struct tagSETUPDATA
    {
        char   szAppName[10]; // name of application
    } SETUPDATA;

SETUPDATA SetUpData;


// Data that can be referenced throughout the
// program but not passed to other instances

HINSTANCE hInst;                        // hInstance of application
HWND      hwnd;                         // hWnd of main window

HANDLE    hdlg;                         // handle of dialog resource

DLGPROC   lpDlgProc;                    // ptr to proc for dialog box
HWND      hDlgBox;                      // handle of dialog box

LOGFONT   MainFontRec;                  // logical font records,...
LOGFONT   CornerFontRec;                // ...which store font data...
LOGFONT   BorlandFontRec;               // ...for different fonts.

COLORREF  FanColor[10];                 // color array for "fan" of text

BOOL      ShadowAll;                    // TRUE iff fan text gets shadows
BOOL      ShowAlignmentMarks;           // TRUE iff we show baselines of fan

#define ROUND(x)  (floor(x + .5))
#define SQR(x)    (pow(x,2))

#define TITLE_LENGTH 30

LPCSTR ArcText = "TrueType";            // text seen along arc of fan
const char* FanText = "Borland C++ for Windows";    // text for leaves of fan
const char* BorlandText = "Borland";    // text for lower right window corner
const int   Radius = 100;               // radius of fan "hub"
const float Deg2Rad = M_PI / 18;        // conversion factor, degrees->radians

// function prototypes

int PASCAL      WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                        LPSTR lpszCmdLine, int cmdShow);

void            Init(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                        LPSTR lpszCmdLine, int cmdShow);
void            InitFirst(HINSTANCE hInstance);
void            InitAdded(HINSTANCE hPrevInstance);
void            InitEvery(HINSTANCE hInstance, int cmdShow);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam);

BOOL CALLBACK  DlgBoxProc(HWND hDlg, UINT message,
                           WPARAM wParam, LPARAM lParam);

void wmCreate(void);
void cmAbout(void);
void cmAlignmentMarks(void);
void cmFonts(HWND);
void cmShadows(void);
void DoPaint(void);
void cm(void);


//*******************************************************************
// WinMain - TrueType main
//
// parameters:
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
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine, int cmdShow)
{
    MSG msg;

    // Go init this application.
    Init(hInstance, hPrevInstance, lpszCmdLine, cmdShow);

    // Get and dispatch messages for this applicaton.
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }  // while

    return(msg.wParam);
}  // end of WinMain()


//*******************************************************************
// Init - init the TrueType demo application
//
// parameters:
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
void Init(HINSTANCE hInstance, HINSTANCE hPrevInstance,
             LPSTR lpszCmdLine, int cmdShow)
{
    
    if (!hPrevInstance)         // if no prev instance, this is first
        InitFirst(hInstance);
    else                        // this is not first instance
        InitAdded(hPrevInstance);

    InitEvery(hInstance, cmdShow);       // init for all instances
}  // end of Init()


//*******************************************************************
// InitFirst - done only for first instance of TrueType demo
//
// parameters:
//             hInstance     - The instance of this instance of this
//                             application.
//
//*******************************************************************
void InitFirst(HINSTANCE hInstance)
{
    WNDCLASS wcTTFClass;

    // Get string from resource with application name.
    LoadString(hInstance, IDS_NAME, (LPSTR) SetUpData.szAppName, 10);

    // Define the window class for this application.
    wcTTFClass.style         = CS_HREDRAW | CS_VREDRAW;
    wcTTFClass.lpfnWndProc   = WndProc;
    wcTTFClass.cbClsExtra    = 0;
    wcTTFClass.cbWndExtra    = 0;
    wcTTFClass.hInstance     = hInstance;
    wcTTFClass.hIcon         = LoadIcon(hInstance, SetUpData.szAppName);
    wcTTFClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcTTFClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcTTFClass.lpszClassName = SetUpData.szAppName;
    wcTTFClass.lpszMenuName  = SetUpData.szAppName;

    // Register the class
    RegisterClass(&wcTTFClass);
}  // end of InitFirst()


//*******************************************************************
// InitAdded - done only for added instances of TrueType demo
//
// parameters:
//             hPrevInstance - The instance of the previous instance
//                             of this application.
//
//*******************************************************************
#pragma argsused
void InitAdded(HINSTANCE hPrevInstance)
{
    // get the results of the initialization of first instance
    GetInstanceData(hPrevInstance, (BYTE*) &SetUpData, sizeof(SETUPDATA));
}  // end of InitAdded()


//*******************************************************************
// InitEvery - done for every instance of TrueType demo program
//
// parameters:
//             hInstance     - The instance of this instance of this
//                             application.
//             cmdShow       - Indicates how the window is to be shown
//                             initially. ie. SW_SHOWNORMAL, SW_HIDE,
//                             SW_MIMIMIZE.
//
//*******************************************************************
#pragma argsused
void InitEvery(HINSTANCE hInstance, int cmdShow)
{
    char title[TITLE_LENGTH+1];

    hInst = hInstance;       // save for use by window procs

    // fetch in the window title from the stringtable resource
    LoadString(hInstance, IDS_TITLE, (LPSTR)title, TITLE_LENGTH);

    // Create applications main window.
    hwnd = CreateWindow(SetUpData.szAppName,    // window class name
                        (LPSTR)title,           // window title
                        WS_OVERLAPPEDWINDOW,    // window style
                        CW_USEDEFAULT,          // x etc.: use defaults
                        CW_USEDEFAULT,          // y
                        CW_USEDEFAULT,          // cx
                        CW_USEDEFAULT,          // cy
                        NULL,                   // no parent for this window
                        NULL,                   // use the class menu
                        hInstance,              // who created this window
                        NULL                    // no parms to pass on
                        );
    ShowWindow(hwnd, cmdShow);
    UpdateWindow(hwnd);
} // end of InitEvery()


//*******************************************************************
// WndProc - handles messages for this application
//
// parameters:
//             hWnd          - The window handle for this message
//             message       - The message number
//             wParam        - The WPARAM parameter for this message
//             lParam        - The LPARAM parameter for this message
//
// returns:
//             depends on message.
//
//*******************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
            wmCreate();
            break;

        case WM_COMMAND:
            switch (wParam)
            {
                case CM_ABOUT:
                    cmAbout();
                    break;

                case CM_EXIT:
                    // Tell windows to destroy our window.
                    DestroyWindow(hWnd);
                    break;

                case CM_ALIGNMENTMARKS:
                    cmAlignmentMarks();
                    break;

                case CM_FONTS:
                    cmFonts(hWnd);
                    break;

                case CM_SHADOWS:
                    cmShadows();
                    break;

                default:
                    break;

            } // switch wParam
            break;
                                                    
        case WM_PAINT:
            DoPaint();
            break;

        case WM_GETMINMAXINFO:
            // Limit the minimum size of the window to 300x300,
            // so the fonts don't get too small
            ((POINT far *)lParam)[3].x = 300;
            ((POINT far *)lParam)[3].y = 300;
            break;

        case WM_DESTROY:
            // This is the end if we were closed by a DestroyWindow call.
            PostQuitMessage(0);
            break;

        default:
            return(DefWindowProc(hWnd, message, wParam, lParam));
    }  // switch message

    return(0L);
}  // end of WndProc()


//*******************************************************************
// DlgBoxProc - handle dialog messages
//
// parameters:
//             hDlg          - The window handle for this message
//             message       - The message number
//             wParam        - The WPARAM parameter for this message 
//             lParam        - The LPARAM parameter for this message
//
//*******************************************************************
#pragma argsused
BOOL CALLBACK DlgBoxProc(HWND hDlg, UINT message,
                           WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
        case WM_SYSCOMMAND:
            // Pass WM_SYSCOMMAND messages on to main window so both
            // main window and dialog box get iconized, minimized etc.
            // in parallel.
            SendMessage(hwnd, message, wParam, lParam);
            break;

        case WM_COMMAND:
            // regardless of the command (only ID_OK should arrive)
            // we want to exit the dialog
            EndDialog(hDlg, TRUE);
            break;

        default:
            return FALSE;
    }  // switch

    return(TRUE);
}  // end of DlgBoxProc()


//*************************************************************************
// DoPaint -- this is the WM_PAINT action routine for the main window.
// It places the TrueType logo in the upper left corner of the window, draws
// a "fan hub" around it, and draws "fan leaves" of text out from the hub.
// These leaves are scaled to reach the bottom and the right edge of the
// window, by defining an ellipse centered on the TrueType logo which just
// touches those two edges.  The text strings span the distance from the
// hub to the ellipse along radial lines, and are both scaled and rotated.
//     Depending on user-set state variables, the ellipse and baselines
// for the text fan may be shown (ShowAlignmentMarks), and/or the text may
// be shadowed (ShadowAll).  Other attributes, such as bolding or
// italicization, may be selected from the font dialog box.
//*************************************************************************
void DoPaint()
{
    PAINTSTRUCT       PaintInfo;
    HDC               hDC;
    LOGFONT           FontRec;
    OUTLINETEXTMETRIC FontMetric;
    int               FontHeight, x, y, j, k;
    WORD              BaseWidth, DesiredExtent, FanTextLen;
    float             Theta;
    LPCSTR            P;
    RECT              R;
    long              TE;
    int               d;

    BeginPaint(hwnd, &PaintInfo);

    hDC = PaintInfo.hdc;
    P = ArcText;
    FanTextLen = strlen(FanText);

    // save device context; easiest way to preserve current state
    SaveDC(hDC);

    // set initial font data (for TrueType logo)
    FontRec = CornerFontRec;
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(128,128,128));
    FontRec.lfHeight = FontRec.lfHeight * 2;
    FontRec.lfWidth = floor(FontRec.lfWidth * 2.1);

    // create the TrueType logo
    SelectObject(hDC, CreateFontIndirect(&FontRec));
    TextOut(hDC, 18, 5, "T", 1);
    SetTextColor(hDC, RGB(0,0,0));
    TextOut(hDC, 32, 13,"T", 1);

    // determine window dimensions & set up fan text parameters
    GetClientRect(hwnd, &R);
    FontRec = MainFontRec;
    DeleteObject(SelectObject(hDC, CreateFontIndirect(&FontRec)));
    GetOutlineTextMetrics(hDC, sizeof(FontMetric), &FontMetric);
    FontHeight = FontMetric.otmTextMetrics.tmHeight;
    SetViewportOrg(hDC, FontHeight+2, 0);
    R.right -= FontHeight+2;
    BaseWidth = LOWORD(GetTextExtent(hDC, FanText, FanTextLen));

    // get a "black brush" for drawing operations
    SelectObject(hDC, GetStockObject(NULL_BRUSH));

    // if we want to show the alignment marks, draw the bounding ellipse
    if (ShowAlignmentMarks)
    {
        Ellipse(hDC, -R.right, -R.bottom, R.right, R.bottom);
    }

    // draw the "hub" of the fan
    Ellipse(hDC, -(Radius-5), -(Radius-5), (Radius-5), Radius-5);
    Ellipse(hDC, -(Radius-10), -(Radius-10), (Radius-10), Radius-10);

    SetTextColor(hDC, FanColor[0]);

    // loop over the "fan leaves"
    for ( d = 27; d <= 36; d++)
    {
        x = ROUND(Radius * cos(d * Deg2Rad));
        y = ROUND(Radius * sin(-d * Deg2Rad)); // -d because y axis is inverted

        Theta = -d * Deg2Rad;
        if (x)
        {
            Theta = atan((R.right / (R.bottom * 1.0)) * (y / (x * 1.0)));
        }

        j = ROUND(R.right * cos(Theta));
        k = ROUND(R.bottom * sin(Theta));

        if (ShowAlignmentMarks)
        {
            MoveTo(hDC, x,y);
            LineTo(hDC, j,k);
        }

        DesiredExtent = ROUND(sqrt(SQR(x*1.0 - j) + SQR(y*1.0 - k))) - 5;
        FontRec = MainFontRec;
        FontRec.lfEscapement = d * 100;
        FontRec.lfWidth = floor((FontMetric.otmTextMetrics.tmAveCharWidth) *
                                (DesiredExtent / (BaseWidth * 1.0)));
        DeleteObject(SelectObject(hDC, CreateFontIndirect(&FontRec)));
        TE = GetTextExtent(hDC, FanText, FanTextLen);

        for ( ;(LOWORD(TE) > DesiredExtent) && (FontRec.lfWidth);
             FontRec.lfWidth-- )
        {
            // Shave off some character width until the string fits 
            DeleteObject(SelectObject(hDC, CreateFontIndirect(&FontRec)));
            TE = GetTextExtent(hDC, FanText, FanTextLen);
        }

        // Expand the string if necessary to make it fit the desired extent 
        if (LOWORD(TE) < DesiredExtent)
          { SetTextJustification(hDC,DesiredExtent - LOWORD(TE), 3); }
        if (ShadowAll)
        {
            SetTextColor(hDC, RGB(0,0,0));
            TextOut(hDC, x+2, y+1, FanText, FanTextLen);
        }
        SetTextColor(hDC, FanColor[d - 27]);
        TextOut(hDC, x, y, FanText, FanTextLen);
        // clear justifier's internal error accumulator 
        SetTextJustification(hDC,0,0);

        if (P[0])
        {
            FontRec = CornerFontRec;
            FontRec.lfEscapement = (d+10) * 100;
            FontRec.lfWidth = 0;
            DeleteObject(SelectObject(hDC, CreateFontIndirect(&FontRec)));
            SetTextColor(hDC, 0);
            x = floor((Radius - FontHeight - 5) * cos(d * Deg2Rad));
            y = floor((Radius - FontHeight - 5) * sin(-d * Deg2Rad));
            TextOut(hDC, x, y, P, 1);
            P++;
        } // if
    } // for d

    // lose the fan font, selecting in the Borland text font
    DeleteObject(SelectObject(hDC, CreateFontIndirect(&BorlandFontRec)));
    TE = GetTextExtent(hDC, BorlandText, strlen(BorlandText));
    SetTextColor(hDC, RGB(0,0,0));
    // write the Borland text in the lower right corner, with a shadow effect
    TextOut(hDC, R.right - LOWORD(TE), R.bottom - HIWORD(TE), BorlandText,
            strlen(BorlandText));
    SetTextColor(hDC, RGB(255,0,0));
    TextOut(hDC, R.right - LOWORD(TE) - 5, R.bottom - HIWORD(TE), BorlandText,
            strlen(BorlandText));

    DeleteObject(SelectObject(hDC, GetStockObject(SYSTEM_FONT)));
    // restore the saved DC; easiest way to reset to entry state
    RestoreDC(hDC, -1);
    EndPaint(hwnd, &PaintInfo);
} // end of DoPaint()


//*************************************************************************
// wmCreate -- action routine for the WM_CREATE message.  This routine
// sets up font record data and colors for use in the painting routines,
// and creates a thunk for the dialog process
//*************************************************************************
void wmCreate(void)
{
    // create the font data for the main font
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

    // structure copy (!) into corner font
    CornerFontRec = MainFontRec;

    // copy again inot Borland font, but switch to Arial typeface, new size
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
} // end of wmCreate()


/*********************************************************************
 cmXxxx routines -- these are action routines for menu commands
*********************************************************************/


//*************************************************************************
// cmShadows -- toggle the ShadowsAll flag, and set the menu checkmark
// as appropriate
//*************************************************************************
void cmShadows(void)
{
    ShadowAll = !ShadowAll;
    if (ShadowAll)
    {
        CheckMenuItem(GetMenu(hwnd), CM_SHADOWS, MF_BYCOMMAND | MF_CHECKED);
    }
    else
    {
        CheckMenuItem(GetMenu(hwnd), CM_SHADOWS, 
                      MF_BYCOMMAND | MF_UNCHECKED);
    }

    // Erase if going Shadow -> no Shadow 
    InvalidateRect(hwnd, NULL, !ShadowAll);
}  // end of cmShadows()


//*************************************************************************
// cmAlignmentMarks -- toggle the state of ShowAlignmentMarks, and toggle
// the checkmark in the menu to match
//*************************************************************************
void cmAlignmentMarks(void)
{
    ShowAlignmentMarks = !ShowAlignmentMarks;
    if (ShowAlignmentMarks)
    {
        CheckMenuItem(GetMenu(hwnd), CM_ALIGNMENTMARKS, 
                      MF_BYCOMMAND | MF_CHECKED);
    }
    else
    {
        CheckMenuItem(GetMenu(hwnd), CM_ALIGNMENTMARKS, 
                      MF_BYCOMMAND | MF_UNCHECKED);
    }

    // Erase if going Marks -> no Marks 
    InvalidateRect(hwnd, NULL, !ShowAlignmentMarks);
}  // end of cmAlignmentMarks()


//*************************************************************************
// cmAbout -- invoke the "About" dialog.  Its return code is ignored, since
// the About dialog doesn't return anything to the program.
//*************************************************************************
void cmAbout(void)
{
    lpDlgProc = (DLGPROC) MakeProcInstance( (FARPROC) DlgBoxProc, hInst);
    DialogBox(hInst, "About", hwnd, lpDlgProc);
    FreeProcInstance ( (FARPROC) lpDlgProc );
}  // end of cmAbout()


//*************************************************************************
// cmFonts -- use the Choose Fonts common dialog to get a new font spec
// from the user.  To do this, we fill out a CHOOSEFONTS structure and
// pass it to the ChooseFonts routine.  Windows 3.1 takes care of the rest!
//*************************************************************************
void cmFonts(HWND hWnd)
{
    CHOOSEFONT CF;
    LOGFONT FontRec = MainFontRec;

    CF.lStructSize    = sizeof(CF);
    CF.hwndOwner      = hWnd;
    CF.Flags          = CF_ANSIONLY | CF_TTONLY | CF_SCREENFONTS |
			CF_INITTOLOGFONTSTRUCT | CF_ENABLETEMPLATE;
    CF.nFontType      = SCREEN_FONTTYPE;
    CF.lpLogFont      = &FontRec;
    CF.nSizeMin       = 20;
    CF.nSizeMax       = 20;
    CF.lpTemplateName = "FontDlg";
    CF.hInstance      = hInst;

    if (ChooseFont(&CF))
    {
        // Only get the font name, weight, and italics;
        // we don't care about size
        strcpy(MainFontRec.lfFaceName, FontRec.lfFaceName);
        MainFontRec.lfWeight = FontRec.lfWeight;
        MainFontRec.lfItalic = FontRec.lfItalic;
        InvalidateRect(hwnd, NULL, TRUE);
    }
}  // end of cmFonts()

