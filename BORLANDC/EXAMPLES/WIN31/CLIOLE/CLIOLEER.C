// Windows 3.1 Demo Program
// Copyright (c) 1992 by Borland International

#define STRICT

#include <windows.h>
#include <ole.h>
#include <process.h>
#pragma hdrstop

#include "clioleer.h"



LPSTR Trans( OLESTATUS status )
{
	switch(status)
	{
		case OLE_OK:
			return "OLE_OK";
		case OLE_WAIT_FOR_RELEASE:
			return "OLE_WAIT_FOR_RELEASE";
		case OLE_BUSY:
			return "OLE_BUSY";
		case OLE_ERROR_PROTECT_ONLY:
			return "OLE_ERROR_PROTECT_ONLY";
		case OLE_ERROR_MEMORY:
			return "OLE_ERROR_MEMORY";
		case OLE_ERROR_STREAM:
			return "OLE_ERROR_STREAM";
		case OLE_ERROR_STATIC:
			return "OLE_ERROR_STATIC";
		case OLE_ERROR_BLANK:
			return "OLE_ERROR_BLANK";
		case OLE_ERROR_DRAW:
			return "OLE_ERROR_DRAW";
		case OLE_ERROR_METAFILE:
			return "OLE_ERROR_METAFILE";
		case OLE_ERROR_ABORT:
			return "OLE_ERROR_ABORT";
		case OLE_ERROR_CLIPBOARD:
			return "OLE_ERROR_CLIPBOARD";
		case OLE_ERROR_FORMAT:
			return "OLE_ERROR_FORMAT";
		case OLE_ERROR_OBJECT:
			return "OLE_ERROR_OBJECT";
		case OLE_ERROR_OPTION:
			return "OLE_ERROR_OPTION";
		case OLE_ERROR_PROTOCOL:
			return "OLE_ERROR_PROTOCOL";
		case OLE_ERROR_ADDRESS:
			return "OLE_ERROR_ADDRESS";
		case OLE_ERROR_NOT_EQUAL:
			return "OLE_ERROR_NOT_EQUAL";
		case OLE_ERROR_HANDLE:
			return "OLE_ERROR_HANDLE";
		case OLE_ERROR_GENERIC:
			return "OLE_ERROR_GENERIC";
		case OLE_ERROR_CLASS:
			return "OLE_ERROR_CLASS";
		case OLE_ERROR_SYNTAX:
			return "OLE_ERROR_SYNTAX";
		case OLE_ERROR_DATATYPE:
			return "OLE_ERROR_DATATYPE";
		case OLE_ERROR_PALETTE:
			return "OLE_ERROR_PALETTE";
		case OLE_ERROR_NOT_LINK:
			return "OLE_ERROR_NOT_LINK";
		case OLE_ERROR_NOT_EMPTY:
			return "OLE_ERROR_NOT_EMPTY";
		case OLE_ERROR_SIZE:
			return "OLE_ERROR_SIZE";
		case OLE_ERROR_DRIVE:
			return "OLE_ERROR_DRIVE";
		case OLE_ERROR_NETWORK:
			return "OLE_ERROR_NETWORK";
		case OLE_ERROR_NAME:
			return "OLE_ERROR_NAME";
		case OLE_ERROR_TEMPLATE:
			return "OLE_ERROR_TEMPLATE";
		case OLE_ERROR_NEW:
			return "OLE_ERROR_NEW";
		case OLE_ERROR_EDIT:
			return "OLE_ERROR_EDIT";
		case OLE_ERROR_OPEN:
			return "OLE_ERROR_OPEN";
		case OLE_ERROR_NOT_OPEN:
			return "OLE_ERROR_NOT_OPEN";
		case OLE_ERROR_LAUNCH:
			return "OLE_ERROR_LAUNCH";
		case OLE_ERROR_COMM:
			return "OLE_ERROR_COMM";
		case OLE_ERROR_TERMINATE:
			return "OLE_ERROR_TERMINATE";
		case OLE_ERROR_COMMAND:
			return "OLE_ERROR_COMMAND";
		case OLE_ERROR_SHOW:
			return "OLE_ERROR_SHOW";
		case OLE_ERROR_DOVERB:
			return "OLE_ERROR_DOVERB";
		case OLE_ERROR_ADVISE_NATIVE:
			return "OLE_ERROR_ADVISE_NATIVE";
		case OLE_ERROR_ADVISE_PICT:
			return "OLE_ERROR_ADVISE_PICT";
		case OLE_ERROR_ADVISE_RENAME:
			return "OLE_ERROR_ADVISE_RENAME";
		case OLE_ERROR_POKE_NATIVE:
			return "OLE_ERROR_POKE_NATIVE";
		case OLE_ERROR_REQUEST_NATIVE:
			return "OLE_ERROR_REQUEST_NATIVE";
		case OLE_ERROR_REQUEST_PICT:
			return "OLE_ERROR_REQUEST_PICT";
		case OLE_ERROR_SERVER_BLOCKED:
			return "OLE_ERROR_SERVER_BLOCKED";
		case OLE_ERROR_REGISTRATION:
			return "OLE_ERROR_REGISTRATION";
		case OLE_ERROR_ALREADY_REGISTERED:
			return "OLE_ERROR_ALREADY_REGISTERED";
		case OLE_ERROR_TASK:
			return "OLE_ERROR_TASK";
		case OLE_ERROR_OUTOFDATE:
			return "OLE_ERROR_OUTOFDATE";
		case OLE_ERROR_CANT_UPDATE_CLIENT:
			return "OLE_ERROR_CANT_UPDATE_CLIENT";
		case OLE_ERROR_UPDATE:
			return "OLE_ERROR_UPDATE";
		case OLE_ERROR_SETDATA_FORMAT:
			return "OLE_ERROR_SETDATA_FORMAT";
		case OLE_ERROR_STATIC_FROM_OTHER_OS:
			return "OLE_ERROR_STATIC_FROM_OTHER_OS";
		case OLE_WARN_DELETE_DATA:
			return "OLE_WARN_DELETE_DATA";
		default:
			return "Not a Status Message";
	}
}


void CheckOleError( OLESTATUS x , unsigned  line , char * file )
// If the message is not OleOk, it gets it translated into the
// appropiate string and pops up a message box giving the file
// and line number.  If The user says no, it exits.
//
{
	if ( x != OLE_OK ) {
		char cap[80];
		char mes[80];
		wsprintf( mes , "%s ( %d )  Ok to Proceed?" , (LPSTR)Trans( x ) , x );
		wsprintf( cap , "OleError - File: %s  Line: %d", (LPSTR)file, line );
		if ( IDNO == MessageBox( GetFocus() ,
			 mes ,
			 cap ,
			 MB_YESNO ) )
		{
			exit( x );
		}
	}
}

void WaitOleNotBusy( OLESTATUS oleStatus, LPOLEOBJECT lpObject ,
			unsigned line, char * file )
{
	MSG msg;
	if ( oleStatus == OLE_WAIT_FOR_RELEASE )
	{
		while ( ( oleStatus = OleQueryReleaseStatus( lpObject )) == OLE_BUSY )
		{
			if ( GetMessage( &msg , NULL, NULL, NULL ))
			{
				TranslateMessage( &msg );
				DispatchMessage(  &msg );
			}
		}
		if ( oleStatus == OLE_OK ) return;
	}
	CheckOleError( oleStatus, line, file );
}
