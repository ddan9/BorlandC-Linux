// ObjectWindows - (C) Copyright 1991,1992 by Borland International

//
// BWCC.H
//


#if !defined(__BWCC_H)
#define __BWCC_H

#if !defined(WORKSHOP_INVOKED)

#if !defined(__WINDOWS_H)
#include <windows.h>

#endif

#endif

// Purpose:    Borland Windows Custom Controls  (BWCC)

#define BWCCVERSION	0x0102   // version 1.02

// from version 1.02 onward BWCCGetversion returns a DWORD
// The low-order word contains the version number
// and the high-order word contains the locale

#define BWCC_LOCALE_US     1
#define BWCC_LOCALE_JAPAN  2


#define BORDLGCLASS	"BorDlg"	// Our Custom Dialog class
#define BORDLGPROP	"FB"		// Borland dialog window uses
					// this property for instance data
					// users should not use a property
					// with this name!

#define IDHELP		998             // Id of help button

// button style definitions:

// the Borland buttons use Windows button styles for button
// type: i.e. BS_PUSHBUTTON/BS_DEFPUSHBUTTON


#define BUTTON_CLASS	"BorBtn"	// Our Bitmap Buttons
#define RADIO_CLASS	"BorRadio"	// Our Radio Buttons
#define CHECK_CLASS	"BorCheck"	// Our Check Boxes

// styles

#define BBS_BITMAP       0x8000L	// this is a bitmap static
#define BBS_DLGPAINT     0x4000L	// used at runtime by dialog class
#define BBS_PARENTNOTIFY 0x2000L	// Notify parent of TAB keys and focus
#define BBS_OWNERDRAW    0x1000L	// let parent paint via WM_DRAWITEM

// messages

#define BBM_SETBITS       ( BM_SETSTYLE + 10)

// notifications

#define BBN_SETFOCUS      ( BN_DOUBLECLICKED + 10)
#define BBN_SETFOCUSMOUSE ( BN_DOUBLECLICKED + 11)
#define BBN_GOTATAB       ( BN_DOUBLECLICKED + 12)
#define BBN_GOTABTAB      ( BN_DOUBLECLICKED + 13)

#define SHADE_CLASS	"BorShade"

#define BSS_GROUP	1L	// group box
#define BSS_HDIP	2L	// horizontal border
#define BSS_VDIP	3L	// vertical border
#define BSS_HBUMP	4L	// horizontal speed bump
#define BSS_VBUMP	5L	// vertical speed bump

#define BSS_DLGERASE	0x8000L // Dialog Window erases for us
#define BSS_DLGPAINT	0x4000L	// Dialog Window paints for us

#define STATIC_CLASS	"BorStatic"	// Our statics

#if !defined(EXPORT)
#define EXPORT _export
#endif

#if defined( __cplusplus )
extern "C" {
#endif  /* __cplusplus */

extern HGLOBAL FAR EXPORT PASCAL SpecialLoadDialog
(
	HINSTANCE hResMod,
	const   LPSTR   lpResName,
	const   DLGPROC fpDlgProc
);

extern HGLOBAL FAR EXPORT PASCAL MangleDialog
(
	HGLOBAL   hDlg,
	HINSTANCE hResources,
	DLGPROC   fpDialogProc
);

extern LRESULT FAR EXPORT PASCAL BWCCDefDlgProc
(
	HWND   hWnd,
	UINT   message,
	WPARAM wParam,
	LPARAM lParam
);
extern LRESULT FAR EXPORT PASCAL BWCCDefWindowProc
(
	HWND   hWnd,
	UINT   message,
	WPARAM wParam,
	LPARAM lParam
);

extern LRESULT FAR EXPORT PASCAL BWCCDefMDIChildProc
(
	HWND   hWnd,
	UINT   message,
	WPARAM wParam,
	LPARAM lParam
);

extern int FAR EXPORT PASCAL BWCCMessageBox
(
	HWND   hWndParent,
	LPCSTR lpText,
	LPCSTR lpCaption,
	UINT   wType
);

extern HBRUSH FAR EXPORT PASCAL BWCCGetPattern( void );
extern DWORD FAR EXPORT PASCAL BWCCGetVersion( void);

#if defined( __cplusplus )
}
#endif  /* __cplusplus */

#endif  /* __BWCC_H */

