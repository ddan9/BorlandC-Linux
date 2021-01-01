// ObjectWindows - (C) Copyright 1992 by Borland International

/* implementation of class TOLEServer */

#include <owl.h>

#define SERVERONLY
#include <ole.h>
#include "shellapi.h"  // registration database

#include "olesrvr.h"

OLESERVERVTBL  TOLEServer::_vtbl;

/*
    WantsToRegister
    ---------------

    displays an action message prompting the user to see if they want to
    register szAppName with the system registration database

    return TRUE if user says YES and FALSE is users says NO

    if user says NO we terminate the app
*/
BOOL
TOLEServer::WantsToRegister ()
{
  char  buf[128];

  wsprintf (buf,
            "\"%s\" is not registered as an OLE server in the system registration \
database. Do you want to register it?",
            (LPSTR) szAppName);

  if (MessageBox (0, buf, szAppName, MB_YESNO | MB_ICONQUESTION) == IDYES)
    return TRUE;

  else {
    //
    // terminate the app
    //
    PostAppMessage (GetCurrentTask(), WM_QUIT, 0, 0);

    //
    // we also need to make sure that the main window doesn't get displayed
    //
    // we have a couple of choices: set "nCmdShow" to SW_HIDE or set "Status"
    // to non-zero. since the user electing not to register isn't really an
    // error, let's set "nCmdShow"
    //
    GetApplicationObject()->nCmdShow = SW_HIDE;
    return FALSE;
  }
}


/*
    RegisterWithDatabase
    --------------------

    register ourselves as an OLE server with the system registration database

    this would typically be done during *installation* of the app and not when
    the app runs...

    NOTE: we first prompt the user to see if they want us to register. if so we
          register and if not we terminate the app
*/
BOOL
TOLEServer::RegisterWithDatabase ()
{
  char  buf[128];
  char  path[MAXPATHLENGTH];
  int   pathLen;

  if (!WantsToRegister())
    return FALSE;

  else {
    wsprintf (buf, ".%s", (LPSTR) szFileExt);
    RegSetValue (HKEY_CLASSES_ROOT,
                 buf, REG_SZ,
                 szClassKey, sizeof (szClassKey) - 1);

    RegSetValue (HKEY_CLASSES_ROOT,
                 szClassKey, REG_SZ,
                 szClassValue, sizeof (szClassValue) - 1);

    //
    // register verb actions EDIT and PLAY with EDIT being the primary verb
    //
    wsprintf (buf, "%s\\protocol\\StdFileEditing\\verb\\0", (LPSTR) szClassKey);
    RegSetValue (HKEY_CLASSES_ROOT, buf, REG_SZ, "Edit", 4);

    wsprintf (buf, "%s\\protocol\\StdFileEditing\\verb\\1", (LPSTR) szClassKey);
    RegSetValue (HKEY_CLASSES_ROOT, buf, REG_SZ, "Play", 4);

    //
    // register a full pathname to the executable with the database
    //
    pathLen = GetModuleFileName (GetModuleHandle (szExeName), path, sizeof (path));
    wsprintf (buf, "%s\\protocol\\StdFileEditing\\server", (LPSTR) szClassKey);
    RegSetValue (HKEY_CLASSES_ROOT, buf, REG_SZ, path, pathLen);

    //
    // inform the user that we have registered as an OLE server by displaying
    // an information message
    //
    wsprintf (buf,
              "\"%s\" successfully registered as an OLE server with the system registration database.",
              (LPSTR) szAppName);

    MessageBox (0, buf, szAppName, MB_OK | MB_ICONINFORMATION);
    return TRUE;
  }
}


/*
    Init
    ----
*/
BOOL
TOLEServer::Init (TOLEApp &app)
{
  OLESTATUS   status;

  lpvtbl = &_vtbl;
  fRelease = FALSE;
  app.pServer = this;

  //
  // since we can't handle multiple documents (MDI), request that we use
  // multiple instances to support multiple objects
  //
	status = OleRegisterServer (szClassKey,
						                  this,
						                  &lhServer,
						                  app.hInstance,
						                  OLE_SERVER_MULTI);

  if (status == OLE_ERROR_CLASS) {
    if (RegisterWithDatabase())
  	  OleRegisterServer (szClassKey,
  						           this,
  						           &lhServer,
  						           app.hInstance,
  						           OLE_SERVER_MULTI);

    else
      return FALSE;
  }

  return TRUE;
}


/*
    constructor
    -----------

    creates an untitled OLE document
*/
TOLEServer::TOLEServer (TOLEApp &app, BOOL embedded)
{
  if (Init (app) && !embedded)
    pDocument = new TOLEDocument (*this);
}


/*
    constructor
    -----------

    creates an OLE document and initializes it from file "szPath"
*/
TOLEServer::TOLEServer (TOLEApp &app, LPSTR szPath)
{
  if (Init (app))
    pDocument = new TOLEDocument (*this, 0, szPath);
}


/*
    callback Create
    -------- ------

    called by the server library when a client application has created a
    new embedded object by calling OleCreate()

    WHAT TO DO:
      - create an *untitled* TOLEDocument of class "lpszClassName" (since we
        only have one class we can ignore the class name) and mark it as dirty
      - associate handle "lhDoc" with the document
      - store the pointer to the TOLEDocument in "lplpOleDoc"
      - return OLE_OK if successful, OLE_ERROR_NEW otherwise

    if your app is an MDI application then you would also allocate a window
    here, but since this app isn't the window is already created...

    "lpszDoc" is the name of the document as it appears in the client
    class. DON'T use this to change the title bar, use what you get when
    the document is sent message "SetHostNames"

    NOTE: since we only have one document we could have created it during
          initialization
*/
OLESTATUS FAR PASCAL _export
TOLEServer::Create (LPOLESERVER         lpOleSvr,
	                  LHSERVERDOC         lhDoc,
	                  LPSTR               /* lpszClassName */,
	                  LPSTR               /* lpszDoc */,
	                  LPOLESERVERDOC FAR *lplpOleDoc)
{
  TOLEServer    *pServer = (TOLEServer *) lpOleSvr;
	TOLEDocument  *pDoc = new TOLEDocument (*pServer, lhDoc, 0, TRUE);

	if (pDoc == NULL)
		return OLE_ERROR_NEW;

  else {
  	*lplpOleDoc = pDoc;
    ((TWindowServer *) GetApplicationObject()->MainWindow)->BeginEmbedding();
  	return OLE_OK;
  }
}


/*
    callback CreateFromTemplate
    -------- ------------------

    called by the server library when a client application has created a
    new linked object specifying a template by calling OleCreateFromTemplate()

    what this really means is that we need to create a document and initialize
    it with the contents of a file...

    "lpszDoc" is the name of the document as it appears in the client
    class. DON'T use this to change the title bar, use what you get when
    the document is sent message "SetHostNames"

    WHAT TO DO:
      - create a TOLEDocument of class "lpszClassName" (since we only have one
        class we can ignore the class name)
      - initialize the document with the contents of file "lpszTemplate"
      - associate handle "lhDoc" with the document
      - store the pointer to the TOLEDocument in "lplpOleDoc"
      - return OLE_OK if successful, OLE_ERROR_TEMPLATE otherwise

    if your app is an MDI application then you would also allocate a window
    here, but since this app isn't the window is already created...

    NOTE: since we only have one document we could have created it during
          initialization
*/
OLESTATUS FAR PASCAL _export
TOLEServer::CreateFromTemplate (LPOLESERVER         lpOleSvr,
                                LHSERVERDOC         lhDoc,
                                LPSTR               /* lpszClassName */,
                                LPSTR               /* lpszDoc */,
                                LPSTR               lpszTemplate,
                                LPOLESERVERDOC FAR *lplpOleDoc)
{
  TOLEServer    *pServer = (TOLEServer *) lpOleSvr;
	TOLEDocument  *pDoc = new TOLEDocument (*pServer, lhDoc, lpszTemplate);

	if (pDoc == NULL)
		return OLE_ERROR_NEW;

  else {
  	*lplpOleDoc = pDoc;
    ((TWindowServer *) GetApplicationObject()->MainWindow)->BeginEmbedding();
  	return OLE_OK;
  }
}


/*
    callback Edit
    -------- ----

    called by the server library when a client application has activated
    an embedded object for editing

    this is exactly like "Create" except that the document will receive a
    "GetData" message to create the object and the object will receive a
    "SetData" message to initialize itself

    "lpszDoc" is the name of the document as it appears in the client
    class. DON'T use this to change the title bar, use what you get when
    the document is sent message "SetHostNames"

    WHAT TO DO:
      - create a TOLEDocument of class "lpszClassName" (since we only have one
        class we can ignore the class name)
      - associate handle "lhDoc" with the document
      - store the pointer to the TOLEDocument in "lplpOleDoc"
      - return OLE_OK if successful, OLE_ERROR_EDIT otherwise
*/
OLESTATUS FAR PASCAL _export
TOLEServer::Edit (LPOLESERVER         lpOleSvr,
	                LHSERVERDOC         lhDoc,
	                LPSTR               /* lpszClassName */,
	                LPSTR               /* lpszDoc */,
	                LPOLESERVERDOC FAR *lplpOleDoc)
{
  TOLEServer    *pServer = (TOLEServer *) lpOleSvr;
  TOLEDocument  *pDoc = new TOLEDocument (*pServer, lhDoc);

  if (pDoc == NULL)
    return OLE_ERROR_EDIT;

  else {
  	*lplpOleDoc = pDoc;
    ((TWindowServer *) GetApplicationObject()->MainWindow)->BeginEmbedding();
    return OLE_OK;
  }
}


/*
    callback Exit
    -------- ----

    we have been instructed by the library to exit immediately because of a
    fatal error

    WHAT TO DO:
      - hide the window to prevent user interaction
      - call OleRevokeServer() and ignore a return of OLE_WAIT_FOR_RELEASE
      - terminate the application immediately
      - return OLE_OK if successful, OLE_ERROR_GENERIC otherwise
*/
OLESTATUS FAR PASCAL _export
TOLEServer::Exit (LPOLESERVER lpOleSvr)
{
  GetApplicationObject()->MainWindow->Show (SW_HIDE);
	OleRevokeServer (((TOLEServer *) lpOleSvr)->lhServer);
  PostAppMessage (GetCurrentTask(), WM_QUIT, 0, 0);
  return OLE_OK;
}


/*
    callback Open
    -------- ----

    user has activated a linked object in an OLE client by calling OleActivate()

    similar to CreateFromTemplate in that we need to create a document,
    initialize it with the contents of file "lpszDoc", and save the file name
    for later use...

    WHAT TO DO:
      - create a TOLEDocument of class "lpszClassName" (since we only have one
        class we can ignore the class name)
      - initialize the document with the contents of file "lpszDoc"
      - associate handle "lhDoc" with the document
      - store the pointer to the TOLEDocument in "lplpOleDoc"
      - save file name "lpszDoc"
      - return OLE_OK if successful, OLE_ERROR_OPEN otherwise
*/
OLESTATUS FAR PASCAL _export
TOLEServer::Open (LPOLESERVER         lpOleSvr,
	                LHSERVERDOC         lhDoc,
	                LPSTR               lpszDoc,
	                LPOLESERVERDOC FAR *lplpOleDoc)
{
  TOLEServer    *pServer = (TOLEServer *) lpOleSvr;
  TOLEDocument  *pDoc = new TOLEDocument (*pServer, lhDoc, lpszDoc);

  if (pDoc == NULL)
    return OLE_ERROR_EDIT;

  else {
    *lplpOleDoc = pDoc;
    return OLE_OK;
  }
}


/*
    callback Release
    -------- -------

    this routine gets called by the server library after the server has
    called OleRevokeServer() and when the DDE conversation with the client has
    been successfully closed

    this tells us that there are no connections to the server, its documents,
    or their objects and that we are free to terminate

    WHAT TO DO:
      - set a flag to indicate that "Release" has been called
      - if the application is hidden and we *haven't* called OleRevokeServer()
        then we *must* terminate by posting a WM_CLOSE message
      - free any resources allocated including documents, but *not* the
        OLESERVER structure
      - return OLE_OK if successful, OLE_ERROR_GENERIC otherwise

    NOTE: this routine is tricky because it is invoked under different
          circumstances

      - user brought up the server and then closes it, which causes us
        to call OleRevokeServer() which means the server will eventually
        receive a "Release" message

      - the server was started to perform an invisible update for a client (i.e.
        the server has always been hidden). in this case the server will receive
        a "Release" message and we must tell ourselves to close because there
        is no user interaction...
*/
OLESTATUS FAR PASCAL _export
TOLEServer::Release (LPOLESERVER lpOleSvr)
{
  //
  // if we haven't been sent a "Release" message yet and our main window is
  // hidden then we post a quit message
  //
  // NOTE: call PostMessage (hWnd, WM_CLOSE, 0, 0) and not PostQuitMessage (0)
  //       because PostQuitMessage() might bypass your application's necessary
  //       cleanup procedures
  //
  TOLEApp     *pApp = (TOLEApp *) GetApplicationObject();
  TOLEServer  *pServer = (TOLEServer *) lpOleSvr;
  BOOL         embedded = pServer->pDocument->type == doctypeEmbedded;

  if (!pServer->fRelease &&
      (embedded || !IsWindowVisible (pApp->MainWindow->HWindow)))
    PostMessage (pApp->MainWindow->HWindow, WM_CLOSE, 0, 0);

  pServer->fRelease = TRUE;
	return OLE_OK;
}


/*
    callback Execute
    -------- -------

    if your app supports DDE execution commands then you would handle this
    event

    since we don't we return OLE_ERROR_COMMAND
*/
OLESTATUS FAR PASCAL _export
TOLEServer::Execute (LPOLESERVER /*lpOleSvr */,
	                   HANDLE      /* hCommands */)
{
	return OLE_ERROR_COMMAND;
}



typedef OLESTATUS FAR PASCAL _export (FAR *LPSRVRCREATE)(LPOLESERVER, LHSERVERDOC, const char far *, const char far *, LPOLESERVERDOC FAR *);
typedef OLESTATUS FAR PASCAL _export (FAR *LPSRVRCREATEFROMTEMPL)(LPOLESERVER,LHSERVERDOC,const char far *, const char far *, const char far *, LPOLESERVERDOC FAR *);
typedef OLESTATUS FAR PASCAL _export (FAR *LPSRVREDIT)(LPOLESERVER,LHSERVERDOC,const char far *, const char far *, LPOLESERVERDOC FAR *);
typedef OLESTATUS FAR PASCAL _export (FAR *LPSRVROPEN)(LPOLESERVER,LHSERVERDOC, const char far *, LPOLESERVERDOC FAR *);
typedef OLESTATUS FAR PASCAL _export (FAR *LPSRVREXECUTE)(LPOLESERVER, HANDLE);
typedef OLESTATUS FAR PASCAL _export (FAR *LPSRVREXIT)(LPOLESERVER);
typedef OLESTATUS FAR PASCAL _export (FAR *LPSRVRRELEASE)(LPOLESERVER);


/*
    InitVTBL
    --------

    create thunks for OLESERVER method callback tables
*/
BOOL
TOLEServer::InitVTBL (HINSTANCE hInstance)
{
	_vtbl.Create = (LPSRVRCREATE) MakeProcInstance ((FARPROC) Create, hInstance);
	_vtbl.CreateFromTemplate = (LPSRVRCREATEFROMTEMPL) MakeProcInstance ((FARPROC) CreateFromTemplate,
                                                                       hInstance);
	_vtbl.Edit = (LPSRVREDIT) MakeProcInstance ((FARPROC) Edit, hInstance);
	_vtbl.Exit = (LPSRVREXIT) MakeProcInstance ((FARPROC) Exit, hInstance);
	_vtbl.Execute = (LPSRVREXECUTE) MakeProcInstance ((FARPROC) Execute, hInstance);
	_vtbl.Open = (LPSRVROPEN) MakeProcInstance ((FARPROC) Open, hInstance);
	_vtbl.Release = (LPSRVRRELEASE) MakeProcInstance ((FARPROC) Release, hInstance);

  return _vtbl.Create != NULL &&
         _vtbl.CreateFromTemplate != NULL &&
         _vtbl.Edit != NULL &&
         _vtbl.Execute != NULL &&
         _vtbl.Open != NULL &&
         _vtbl.Release != NULL;
}

