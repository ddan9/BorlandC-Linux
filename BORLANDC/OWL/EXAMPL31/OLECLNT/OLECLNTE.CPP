// ObjectWindows - (C) Copyright 1992 by Borland International
//
// oleclnte.cpp

// see comments in oleclnte.h for a description of classes.

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <ole.h>
#pragma hdrstop

#include "oleclnte.h"

LPSTR TOleStatusTrans::Default() { return "Not a Status Message"; }
LPSTR TOleStatusTrans::Ok() { return "OLE_OK"; }
LPSTR TOleStatusTrans::WaitForRelease() { return "OLE_WAIT_FOR_RELEASE"; }
LPSTR TOleStatusTrans::Busy() { return "OLE_BUSY"; }
LPSTR TOleStatusTrans::ErrorProtectOnly() { return "OLE_ERROR_PROTECT_ONLY"; }
LPSTR TOleStatusTrans::ErrorMemory() { return "OLE_ERROR_MEMORY"; }
LPSTR TOleStatusTrans::ErrorStream() { return "OLE_ERROR_STREAM"; }
LPSTR TOleStatusTrans::ErrorStatic() { return "OLE_ERROR_STATIC"; }
LPSTR TOleStatusTrans::ErrorBlank()   { return "OLE_ERROR_BLANK"; }
LPSTR TOleStatusTrans::ErrorDraw()    { return "OLE_ERROR_DRAW"; }
LPSTR TOleStatusTrans::ErrorMetafile() { return "OLE_ERROR_METAFILE"; }
LPSTR TOleStatusTrans::ErrorAbort()   { return "OLE_ERROR_ABORT"; }
LPSTR TOleStatusTrans::ErrorClipboard() { return "OLE_ERROR_CLIPBOARD"; }
LPSTR TOleStatusTrans::ErrorFormat()  { return "OLE_ERROR_FORMAT"; }
LPSTR TOleStatusTrans::ErrorObject() { return "OLE_ERROR_OBJECT"; }
LPSTR TOleStatusTrans::ErrorOption() { return "OLE_ERROR_OPTION"; }
LPSTR TOleStatusTrans::ErrorProtocal() { return "OLE_ERROR_PROTOCOL"; }
LPSTR TOleStatusTrans::ErrorAddress() { return "OLE_ERROR_ADDRESS"; }
LPSTR TOleStatusTrans::ErrorNotEqual() { return "OLE_ERROR_NOT_EQUAL"; }
LPSTR TOleStatusTrans::ErrorHandle()    { return "OLE_ERROR_HANDLE"; }
LPSTR TOleStatusTrans::ErrorGeneric() { return "OLE_ERROR_GENERIC"; }
LPSTR TOleStatusTrans::ErrorClass()    { return "OLE_ERROR_CLASS"; }
LPSTR TOleStatusTrans::ErrorSyntax() { return "OLE_ERROR_SYNTAX"; }
LPSTR TOleStatusTrans::ErrorDataType() { return "OLE_ERROR_DATATYPE"; }
LPSTR TOleStatusTrans::ErrorPalette() { return "OLE_ERROR_PALETTE"; }
LPSTR TOleStatusTrans::ErrorNotLink() { return "OLE_ERROR_NOT_LINK"; }
LPSTR TOleStatusTrans::ErrorNotEmpty() { return "OLE_ERROR_NOT_EMPTY"; }
LPSTR TOleStatusTrans::ErrorSize() { return "OLE_ERROR_SIZE"; }
LPSTR TOleStatusTrans::ErrorDrive() { return "OLE_ERROR_DRIVE"; }
LPSTR TOleStatusTrans::ErrorNetwork() { return "OLE_ERROR_NETWORK"; }
LPSTR TOleStatusTrans::ErrorName() { return "OLE_ERROR_NAME"; }
LPSTR TOleStatusTrans::ErrorTemplate() { return "OLE_ERROR_TEMPLATE"; }
LPSTR TOleStatusTrans::ErrorNew() { return "OLE_ERROR_NEW"; }
LPSTR TOleStatusTrans::ErrorEdt() { return "OLE_ERROR_EDIT"; }
LPSTR TOleStatusTrans::ErrorOpen() { return "OLE_ERROR_OPEN"; }
LPSTR TOleStatusTrans::ErrorNotOpen() { return "OLE_ERROR_NOT_OPEN"; }
LPSTR TOleStatusTrans::ErrorLaunch() { return "OLE_ERROR_LAUNCH"; }
LPSTR TOleStatusTrans::ErrorComm() { return "OLE_ERROR_COMM"; }
LPSTR TOleStatusTrans::ErrorTerminate() { return "OLE_ERROR_TERMINATE"; }
LPSTR TOleStatusTrans::ErrorCommand() { return "OLE_ERROR_COMMAND"; }
LPSTR TOleStatusTrans::ErrorShow() { return "OLE_ERROR_SHOW"; }
LPSTR TOleStatusTrans::ErrorDoverb() { return "OLE_ERROR_DOVERB"; }
LPSTR TOleStatusTrans::ErrorAdviseNatvie() { return "OLE_ERROR_ADVISE_NATIVE"; }
LPSTR TOleStatusTrans::ErrorAdvisePict() { return "OLE_ERROR_ADVISE_PICT"; }
LPSTR TOleStatusTrans::ErrorAdviseRename() { return "OLE_ERROR_ADVISE_RENAME"; }
LPSTR TOleStatusTrans::ErrorPokeNative() { return "OLE_ERROR_POKE_NATIVE"; }
LPSTR TOleStatusTrans::ErrorRequestNative() { return "OLE_ERROR_REQUEST_NATIVE"; }
LPSTR TOleStatusTrans::ErrorRequestPict() { return "OLE_ERROR_REQUEST_PICT"; }
LPSTR TOleStatusTrans::ErrorServerBlocked() { return "OLE_ERROR_SERVER_BLOCKED"; }
LPSTR TOleStatusTrans::ErrorRegistration() { return "OLE_ERROR_REGISTRATION"; }
LPSTR TOleStatusTrans::ErrorAlreadyRegistered() { return "OLE_ERROR_ALREADY_REGISTERED"; }
LPSTR TOleStatusTrans::ErrorTask() { return "OLE_ERROR_TASK"; }
LPSTR TOleStatusTrans::ErrorOutOfDate() { return "OLE_ERROR_OUTOFDATE"; }
LPSTR TOleStatusTrans::ErrorCantUpdateClient() { return "OLE_ERROR_CANT_UPDATE_CLIENT"; }
LPSTR TOleStatusTrans::ErrorUpdate() { return "OLE_ERROR_UPDATE"; }
LPSTR TOleStatusTrans::ErrorSetdataFormat() { return "OLE_ERROR_SETDATA_FORMAT"; }
LPSTR TOleStatusTrans::ErrorStaticFromOtherOs() { return "OLE_ERROR_STATIC_FROM_OTHER_OS"; }
LPSTR TOleStatusTrans::WarnDeleteData() { return "OLE_WARN_DELETE_DATA"; }

typedef LPSTR ( TOleStatusTrans::*ConFP )();
typedef LPSTR ( *FP)();

/* _DDVTdispathNULL is a function in the RTL.  There are two versions,
one for far vtables, one for near.  They both return a pointer to a
pointer to the address of the DDVT function, or NULL if one is not found.
*/

#ifdef FARVTBL

#define GetVptr(thisPtr)    (*(void far **)(thisPtr))
FP far * _DDVTdispatchNULL(void far *, int);

#else

#define GetVptr( thisPtr)   (*(void near **)(thisPtr))
FP far * _DDVTdispatchNULL(void near *, int);

#endif


LPSTR TOleStatusTrans::Trans( OLESTATUS x )
{
	ConFP theDDVTfoo;              // final location for DDVT foo
	union {
		FP    fp;                   // since the RTL returns a basic
		ConFP memberFP;             // function pointer address, we
	} temp;                         // use a union to transform to
					// member function pointer.

	FP fp = *_DDVTdispatchNULL( GetVptr( this ) , x );
	if ( fp )
	{
		temp.fp = fp;
		theDDVTfoo = temp.memberFP;
	}
	else theDDVTfoo = &TOleStatusTrans::Default;

	return (this->*theDDVTfoo)();
}



void CheckOleError( OLESTATUS x , unsigned  line , char * file )
// If the message is not OleOk, it gets it translated into the
// appropiate string and pops up a message box giving the file
// and line number.  If The user says no, it exits.
//
{
	static TOleStatusTrans OleStatusTrans;

	if ( x != OLE_OK ) {
		char cap[80];
		char mes[80];
		sprintf( mes , "%s ( %d )  Ok to Proceed?" , OleStatusTrans.Trans( x ) , x );
		sprintf( cap , "OleError - File: %s  Line: %d", file, line );
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
		if ( oleStatus == OLE_OK || oleStatus == OLE_ERROR_OBJECT ) return;
	}
	CheckOleError( oleStatus, line, file );
}
