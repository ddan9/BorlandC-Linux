// ObjectWindows - (C) Copyright 1992 by Borland International
//
// oleclnte.h

// Defines the macros check and wait.
// check is used to see if the last Ole function completed
// successfully.
// wait is used to go into a Message-Dispatch loop until
// an Object is released.  It also checks that the last Ole
// operation was successful.  They are implemented as macros to
// take advantage of the __LINE__ and __FILE__ macros which
// the compiler pre-defines.  This allows one to see which
// line in the source code an error occured at.  They put up
// a message box to display the symbolic constant for the
// error which ole.h declared.  The comments in ole.h sometimes
// give more information on what an error code meant.

// Warning: Putting up a MessageBox during the operation of Ole
// functions may be a little dangerous, as the message loop for
// the MessageBox is in Windows.  This means that any un-attended
// messages between the OLE libraries must go through that message
// loop.  A Safer thing to do is output debug strings to a
// monochrome monitor, if available.  For the purposes of this
// example, it should not be a problem.

#define check( x ) CheckOleError( x , __LINE__ , __FILE__ )
	//if not OLE_OK, CheckOleError will put up a message
	//box, which could be bad for Ole message dispatching

#define wait( x , y) WaitOleNotBusy(  x , y , __LINE__, __FILE__ )

void CheckOleError( OLESTATUS , unsigned , char * );
void WaitOleNotBusy( OLESTATUS , LPOLEOBJECT , unsigned, char * );


#define FARVTBL

// TOleStatusTrans is a class used to translate Ole error codes
// to their symbolic constants.  It also serves as an example of using
// Dynamic Dispatching outside of the TWindowsObject hierarchy.
// The class is made huge to ensure that the vtable pointer will
// be far regardless of wether or not a static or dynamic link is
// done with the class.  FARVTBL is defined to make this known
// to the source code which uses this header file.
// The C ole client example uses a switch statement to translate
// error codes to their symbolic constants.

class huge TOleStatusTrans {
public:
	virtual LPSTR Ok() = [ OLE_OK ];
	virtual LPSTR WaitForRelease() = [ OLE_WAIT_FOR_RELEASE ];
	virtual LPSTR Busy() = [ OLE_BUSY ];
	virtual LPSTR ErrorProtectOnly() = [ OLE_ERROR_PROTECT_ONLY ];
	virtual LPSTR ErrorMemory() = [ OLE_ERROR_MEMORY ];
	virtual LPSTR ErrorStream() = [ OLE_ERROR_STREAM ];
	virtual LPSTR ErrorStatic() = [ OLE_ERROR_STATIC ];
	virtual LPSTR ErrorBlank() =  [ OLE_ERROR_BLANK ];
	virtual LPSTR ErrorDraw() =   [ OLE_ERROR_DRAW ];
	virtual LPSTR ErrorMetafile() = [ OLE_ERROR_METAFILE ];
	virtual LPSTR ErrorAbort() =  [ OLE_ERROR_ABORT ];
	virtual LPSTR ErrorClipboard() = [ OLE_ERROR_CLIPBOARD ];
	virtual LPSTR ErrorFormat() =  [ OLE_ERROR_FORMAT ];
	virtual LPSTR ErrorObject() = [ OLE_ERROR_OBJECT ];
	virtual LPSTR ErrorOption() = [ OLE_ERROR_OPTION ];
	virtual LPSTR ErrorProtocal() = [ OLE_ERROR_PROTOCOL ];
	virtual LPSTR ErrorAddress() = [ OLE_ERROR_ADDRESS ];
	virtual LPSTR ErrorNotEqual() = [ OLE_ERROR_NOT_EQUAL ];
	virtual LPSTR ErrorHandle() =   [ OLE_ERROR_HANDLE ];
	virtual LPSTR ErrorGeneric() = [ OLE_ERROR_GENERIC ];
	virtual LPSTR ErrorClass() =   [ OLE_ERROR_CLASS ];
	virtual LPSTR ErrorSyntax() = [ OLE_ERROR_SYNTAX ];
	virtual LPSTR ErrorDataType() = [ OLE_ERROR_DATATYPE ];
	virtual LPSTR ErrorPalette() = [ OLE_ERROR_PALETTE ];
	virtual LPSTR ErrorNotLink() = [ OLE_ERROR_NOT_LINK ];
	virtual LPSTR ErrorNotEmpty() = [ OLE_ERROR_NOT_EMPTY ];
	virtual LPSTR ErrorSize() = [ OLE_ERROR_SIZE ];
	virtual LPSTR ErrorDrive() = [ OLE_ERROR_DRIVE ];
	virtual LPSTR ErrorNetwork() = [ OLE_ERROR_NETWORK ];
	virtual LPSTR ErrorName() = [ OLE_ERROR_NAME ];
	virtual LPSTR ErrorTemplate() = [ OLE_ERROR_TEMPLATE ];
	virtual LPSTR ErrorNew() = [ OLE_ERROR_NEW ];
	virtual LPSTR ErrorEdt() = [ OLE_ERROR_EDIT ];
	virtual LPSTR ErrorOpen() = [ OLE_ERROR_OPEN ];
	virtual LPSTR ErrorNotOpen() = [ OLE_ERROR_NOT_OPEN ];
	virtual LPSTR ErrorLaunch() = [ OLE_ERROR_LAUNCH ];
	virtual LPSTR ErrorComm() = [ OLE_ERROR_COMM ];
	virtual LPSTR ErrorTerminate() = [ OLE_ERROR_TERMINATE ];
	virtual LPSTR ErrorCommand() = [ OLE_ERROR_COMMAND ];
	virtual LPSTR ErrorShow() = [ OLE_ERROR_SHOW ];
	virtual LPSTR ErrorDoverb() = [ OLE_ERROR_DOVERB ];
	virtual LPSTR ErrorAdviseNatvie() = [ OLE_ERROR_ADVISE_NATIVE ];
	virtual LPSTR ErrorAdvisePict() = [ OLE_ERROR_ADVISE_PICT ];
	virtual LPSTR ErrorAdviseRename() =[ OLE_ERROR_ADVISE_RENAME ];
	virtual LPSTR ErrorPokeNative() = [ OLE_ERROR_POKE_NATIVE ];
	virtual LPSTR ErrorRequestNative() = [ OLE_ERROR_REQUEST_NATIVE ];
	virtual LPSTR ErrorRequestPict() = [ OLE_ERROR_REQUEST_PICT ];
	virtual LPSTR ErrorServerBlocked() = [ OLE_ERROR_SERVER_BLOCKED ];
	virtual LPSTR ErrorRegistration() = [ OLE_ERROR_REGISTRATION ];
	virtual LPSTR ErrorAlreadyRegistered() = [ OLE_ERROR_ALREADY_REGISTERED ];
	virtual LPSTR ErrorTask() = [ OLE_ERROR_TASK ];
	virtual LPSTR ErrorOutOfDate() = [ OLE_ERROR_OUTOFDATE ];
	virtual LPSTR ErrorCantUpdateClient() = [ OLE_ERROR_CANT_UPDATE_CLIENT ];
	virtual LPSTR ErrorUpdate() = [ OLE_ERROR_UPDATE ];
	virtual LPSTR ErrorSetdataFormat() = [ OLE_ERROR_SETDATA_FORMAT ];
	virtual LPSTR ErrorStaticFromOtherOs() = [ OLE_ERROR_STATIC_FROM_OTHER_OS ];
	virtual LPSTR WarnDeleteData() = [ OLE_WARN_DELETE_DATA ];
	LPSTR Trans( OLESTATUS );
	LPSTR Default();
};


