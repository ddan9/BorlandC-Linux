// ObjectWindows - (C) Copyright 1992 by Borland International

/* This OWL example demonstrates how to make an application PenWindows
 * sensitive.
 *
 * If this application is not running under PenWindows then the application
 * works like a traditional Windows application with a TextEdit box.
 *
 * Following the instructions in WINPEN.DOC, in the DOC directory, on how to
 * modify your SYSTEM.INI file to support PenWindows with or without a pen.
 * The mouse should only be used as the stylus for testing; it is almost
 * always inferior to a pen/tablet hardware configuration. It will also
 * produce poor recognition results.
 *
 * This is the simplest way to make an application pen aware.  The steps are:
 *
 *     1. Check for PenWindows
 *     2. Register the application as a pen application, if PenWindow exists
 *     3. Create two different dialog boxes depending on whether the application
 *        is running under Windows or PenWindows.  Under Pen Windows a BEDIT
 *        control is used for text recognition and entry.
 */

#include <owl.h>
#include <button.h>
#include <dialog.h>

#include <penwin.h>

#include "owlpen.h"

typedef VOID (FAR PASCAL *LPREGPENAPP)(WORD, BOOL);

class TTestApp : public TApplication
{
public:
    LPREGPENAPP RegisterPenApp;
    HANDLE hPenWin;

    TTestApp(LPSTR AName, HINSTANCE hInstance, HINSTANCE hPrevInstance,
             LPSTR lpCmdLine, int nCmdShow)
        : TApplication(AName, hInstance, hPrevInstance, lpCmdLine, nCmdShow) {};
    virtual void InitMainWindow();
};

class TTestWindow : public TWindow
{
public:
    char EditText[80];
    HANDLE hPenWindow;

    TTestWindow(PTWindowsObject AParent, LPSTR ATitle, HANDLE hPenWin);
    virtual void HandleButtonMsg(RTMessage Msg) = [ID_FIRST + ID_BUTTON];
    virtual void Paint (HDC PaintDC, PAINTSTRUCT _FAR &PaintInfo);
    void SetEditText (LPSTR lpText);
};

_CLASSDEF(TDialogTextentry)
class TDialogTextentry : public TDialog
{
public:
    TDialogTextentry (PTWindowsObject Parent, int iResId);
    virtual void HandleShowButton (RTMessage Msg) = [ID_FIRST + ID_SHOW];
};


TTestWindow::TTestWindow(PTWindowsObject AParent, LPSTR ATitle, HANDLE hPenWin)
          : TWindow(AParent, ATitle)
{
    hPenWindow = hPenWin;

    //display a line of text, showing (Pen)Windows is installed
    if (hPenWindow)
        lstrcpy (EditText, "PenWindows Installed");
    else
        lstrcpy (EditText, "PenWindows is not Installed");

    Attr.X = 100;
    Attr.Y = 100;
    Attr.W = 400;
    Attr.H = 300;
    new TButton(this, ID_BUTTON, "Text Entry",0, 0, 100, 24, FALSE);
}

void TTestWindow::Paint(HDC PaintDC, PAINTSTRUCT _FAR &)
{
    TextOut (PaintDC, 150, 2, EditText, lstrlen (EditText));
}

void TTestWindow::SetEditText (LPSTR lpText)
{
    lstrcpy (EditText, lpText);
}

TDialogTextentry::TDialogTextentry (PTWindowsObject Parent, int iResId)
                    :TDialog (Parent, iResId)
{
}

// response function to SHOW button, get text and show it in the lower control
void TDialogTextentry::HandleShowButton (RTMessage)
{
    char Buffer[80];

    GetDlgItemText (HWindow, ID_EDIT, Buffer, 80);
    SetDlgItemText (HWindow, ID_TEXT, Buffer);
}

// open a dialogbox, depending on (Pen)Windows running or not
// You may look at OWLPEN.RC file and see that BEDIT is used in
// PENTEXTENTRY_DIALOG
void TTestWindow::HandleButtonMsg(RTMessage)
{
    GetModule()->ExecDialog(new TDialogTextentry (this, hPenWindow ?
                                   PENTEXTENTRY_DIALOG : TEXTENTRY_DIALOG));
}


void TTestApp::InitMainWindow()
{
    //Is PenWindows running?
    if ((hPenWin = (HINSTANCE) GetSystemMetrics (SM_PENWINDOWS)) != 0) {

        // PenWindows is now running, Get adress of RegisterPenApp() using
        // runtime dynamic linking
        if ((RegisterPenApp = (LPREGPENAPP)GetProcAddress ((HINSTANCE)hPenWin,
	                          "RegisterPenApp")) != 0) {

            // Call RegisterPenApp() to indicate that we are a PenWindows
            // application
            (*RegisterPenApp)(RPA_DEFAULT, TRUE);
        }
    }

    MainWindow = new TTestWindow(NULL, Name, hPenWin);
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    TTestApp TestApp("This Owl is pen aware", hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    TestApp.Run();
    return TestApp.Status;
}
