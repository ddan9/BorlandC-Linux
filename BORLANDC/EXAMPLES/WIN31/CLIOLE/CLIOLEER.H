// Windows 3.1 Demo Program
// Copyright (c) 1992 by Borland International

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


void CheckOleError( OLESTATUS , unsigned , char * );
void WaitOleNotBusy( OLESTATUS , LPOLEOBJECT , unsigned, char * );

#define check( x ) CheckOleError( x , __LINE__ , __FILE__ )
	//if not OLE_OK, CheckOleError will put up a message
	//box, which could be bad for Ole message dispatching

#define wait( x , y) WaitOleNotBusy(  x , y , __LINE__, __FILE__ )

