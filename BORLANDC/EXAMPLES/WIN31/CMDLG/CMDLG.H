// Borland C++ Windows - (C) Copyright 1992 by Borland International

// FILE : cmdlg.h

#define STRICT

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <commdlg.h>
#include "cmdlgr.h"

long FAR PASCAL _export MainWndProc(HWND, unsigned, WORD, LONG);
BOOL InitApplication(HINSTANCE);
