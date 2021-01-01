// ObjectWindows - (C) Copyright 1992 by Borland International

/* implementation of classes TAppOleServer & TWindowServer */

#include <owl.h>

#define SERVERONLY
#include <ole.h>

#include <ctype.h>
#include "olesrvr.h"

#define szEmbedding   "Embedding"



/*
    constructor
    -----------
*/
TWindowServer::TWindowServer (PTWindowsObject Parent,
                              LPSTR           Title) : TWindow (Parent, Title)
{
  AssignMenu ("MenuBar");

	Attr.Style = WS_OVERLAPPEDWINDOW;
	Attr.X = 100;
	Attr.Y = 100;
	Attr.W = 250;
	Attr.H = 250;
}


/*
    About
    -----

    Help/About menu item
*/
void
TWindowServer::About (RTMessage /* Msg */)
{
  GetApplication()->ExecDialog (new TDialog (this, "AboutBox"));
}


/*
    SaveChangesPrompt
    -----------------

    the user has requested File/New, File/Open, or File/Exit

    prompt the user to save changes in the document and return whether the
    pending operation (new/open/exit) should be executed or canceled
*/
FILEIOSTATUS
TWindowServer::SaveChangesPrompt ()
{
  TOLEApp       *pApp = (TOLEApp *) GetApplication();
  TOLEDocument  *pDoc = pApp->pServer->pDocument;
  int            outcome = IDYES;

  if (pDoc->fDirty) {
    char   buf[128];

    if (pDoc->type == doctypeEmbedded)
      wsprintf (buf,
                "Embedded object %s has changed. Do you want to update?",
                (LPSTR) pDoc->szName);

    else
      wsprintf (buf, "Do you want to save changes to %s?", (LPSTR) pDoc->szName);

    outcome = MessageBox (HWindow, buf, szAppName, MB_ICONQUESTION | MB_YESNOCANCEL);

    if (outcome == IDYES)
      if (pDoc->type == doctypeEmbedded)
        OleSavedServerDoc (pDoc->lhDoc);

      else
          pDoc->SaveDoc();
  }

  if (outcome != IDCANCEL) {
    if (OleRevokeServerDoc (pDoc->lhDoc) == OLE_WAIT_FOR_RELEASE) {
      //
      // server library is in the process of closing down connections to
      // the document. wait until it is finished (flag "fDirty" becomes TRUE)
      // before we re-use the document space...
      //
      pApp->Wait (pDoc->fRelease);
    }

    pDoc->lhDoc = 0;

    if (pDoc->type == doctypeEmbedded)
      EndEmbedding();
  }

  return outcome == IDCANCEL ? fiCancel : fiExecute;
}

/* Prompt the user for changes and initiate application shutdown by
 * calling OleRevokeServer()
 *
 * OleRevokeServer() automatically revokes any documents which revokes
 *   any objects
 */
BOOL TWindowServer::CanClose()
{
   if (SaveChangesPrompt() == fiExecute) {
    TOLEApp     *pApp = (TOLEApp *) GetApplication();
    TOLEServer  *pServer = pApp->pServer;

    if (OleRevokeServer (pServer->lhServer) == OLE_WAIT_FOR_RELEASE) {
      //
      // server library is in the process of closing down connections to
      // the server. wait until it is finished (flag "fDirty" becomes TRUE)
      // before we terminate
      //
      pApp->Wait (pServer->fRelease);
    }
    return(TRUE);
  }
  else
    return(FALSE);
}

/*
    New
    ---

    File/New menu item
*/
void
TWindowServer::New (RTMessage /* Msg */)
{
  if (SaveChangesPrompt() == fiExecute) {
    //
    // make the new document (we reuse the same objects)
    //
    ((TOLEApp *) GetApplication())->pServer->pDocument->Reset (0);
  }
}


/*
    Open
    ----

    File/Open menu item
*/
void
TWindowServer::Open (RTMessage /* Msg */)
{
  if (SaveChangesPrompt() == fiExecute) {
    char           path[MAXPATHLENGTH];
    TOLEDocument  *pDoc = ((TOLEApp *) GetApplication())->pServer->pDocument;

    pDoc->Reset (TOLEDocument::PromptForOpenFileName (path) ? path : 0);
  }
}


/*
    Save
    ----

    File/Save menu item

    NOTE: only for stand-alone mode...
*/
void
TWindowServer::Save (RTMessage /* Msg */)
{
  ((TOLEApp *) GetApplication())->pServer->pDocument->SaveDoc();
}


/*
    SaveAs
    ------

    File/Save As menu item
*/
void
TWindowServer::SaveAs (RTMessage /* Msg */)
{
  ((TOLEApp *) GetApplication())->pServer->pDocument->SaveAs();
}


/*
    Update
    ------

    File/Update menu item

    NOTE: only for embedding mode...
*/
void
TWindowServer::Update (RTMessage /* Msg */)
{
  TOLEDocument  *pDoc = ((TOLEApp *) GetApplication())->pServer->pDocument;

  //
  // notify the server library that the embedded document has changed
  //
  OleSavedServerDoc (pDoc->lhDoc);

  //
  // reset the doc's "fDirty" flag to FALSE
  //
  pDoc->fDirty = FALSE;
}


/*
    Copy
    ----

    Edit/Copy menu item

    WHAT IT DOES:
      - open and empty the clipboard
      - put the data formats on the clipboard
      - close the clipboard

    NOTE: since this app only has one object we don't support "Cut" and
          "Delete", but your app might want to
*/
void
TWindowServer::Copy (RTMessage /* Msg */)
{
  TOLEApp     *pApp = (TOLEApp *) GetApplication();
  TOLEObject  *pObject = pApp->pServer->pDocument->pObject;
  HANDLE       handle;

  if (OpenClipboard (HWindow)) {
    EmptyClipboard();

    //
    // server applications are responsible for placing the data formats on the
    // clipboard in most important order first
    //
    // here is the standard ordering:
    //    1. Application-specific data
    //    2. Native
    //    3. OwnerLink
    //    4. CF_METAFILEPICT
    //    5. CF_BITMAP
    //    6. ObjectLink
    //    7. Any other data
    //
    // add Native first...
    //
    if ((handle = pObject->GetNativeData()) != NULL)
      SetClipboardData (pApp->cfNative, handle);

    //
    // in order for the object to be embedded we must also identify the
    // owner of the object using "OwnerLink" data
    //
    if ((handle = pObject->GetLinkData()) != NULL)
      SetClipboardData (pApp->cfOwnerLink, handle);

    //
    // now offer at least one presentation format
    //
    // if the server doesn't have an object handler DLL then it must provide
    // a metafile
    //
    if ((handle = pObject->GetMetafilePicture()) != NULL)
      SetClipboardData (CF_METAFILEPICT, handle);

    //
    // now offer bitmap
    //
    if ((handle = pObject->GetBitmapData()) != NULL)
      SetClipboardData (CF_BITMAP, handle);

    //
    // if the document type is a file then we can offer "ObjectLink"
    //
    if (pApp->pServer->pDocument->type == doctypeFromFile)
      if ((handle = pObject->GetLinkData()) != NULL)
        SetClipboardData (pApp->cfObjectLink, handle);

    //
    // now close the clipboard
    //
    CloseClipboard ();
  }
}


/*
    ShapeChange
    -----------

    user has selected a menu item from the "Shape" menu

    check the new menu item, uncheck the previous menu item, change the
    selected object's type, repaint the damaged area, and check the menu items
    to see if they should be enabled/disabled
*/
void
TWindowServer::ShapeChange (NATIVETYPE newType)
{
  TOLEDocument  *pDoc = ((TOLEApp *) GetApplication())->pServer->pDocument;
  TOLEObject    *pObject = pDoc->pObject;
  NATIVETYPE     oldType = pObject->GetType();

  if (newType != oldType) {
    RECT   r = {OBJX, OBJY, OBJX + OBJWIDTH, OBJY + OBJHEIGHT};

    //
    // change the object's type which marks the document as "dirty" and
    // notifies each linked object of the change
    //
    pObject->SetType (newType);

    InvalidateRect (HWindow, &r, TRUE);

    //
    // now remove the check from the previous menu item and check the new
    // selection
    //
    HMENU  hMenu = GetMenu (HWindow);

    CheckMenuItem (hMenu, oldType, MF_UNCHECKED);
    CheckMenuItem (hMenu, newType, MF_CHECKED);
  }
}


/*
    WmCommand
    ---------

    rather than have a message response function for each menu item on the
    "Shape" menu we catch the commands here instead

    other commands are passed to our inherited method
*/
void
TWindowServer::WMCommand (RTMessage Msg)
{
  if (Msg.WParam >= IDM_ELLIPSE && Msg.WParam <= IDM_TRIANGLE)
    ShapeChange ((NATIVETYPE) Msg.WParam);

  else
    TWindow::WMCommand (Msg);
}


/*
    BeginEmbedding
    --------------

    change the File/Save As... menu item to File/Save Copy As...
*/
void
TWindowServer::BeginEmbedding ()
{
  HMENU  hMenu = GetMenu (HWindow);

  ModifyMenu (hMenu, IDM_SAVEAS, MF_BYCOMMAND | MF_STRING,
              IDM_SAVEAS, "Save Copy &As...");
}


/*
    UpdateFileMenu
    --------------

    change File/Save and File/Exit
*/
void
TWindowServer::UpdateFileMenu (LPSTR szDocName)
{
  HMENU  hMenu = GetMenu (HWindow);
  char   buf[128];

  //
  // change File/Save to File/Update <Client Document>
  //
  wsprintf (buf, "&Update %s", szDocName);
  ModifyMenu (hMenu, IDM_SAVE, MF_BYCOMMAND | MF_STRING, IDM_UPDATE, buf);

  //
  // change File/Exit to File/Exit & Return to <Client Document>
  //
  wsprintf (buf, "E&xit && Return to %s", szDocName);
  ModifyMenu (hMenu, CM_EXIT, MF_BYCOMMAND | MF_STRING, CM_EXIT, buf);
}


/*
    EndEmbedding
    ------------

    change File/Save Copy As..., File/Exit & Return, and File/Update
*/
void
TWindowServer::EndEmbedding ()
{
  HMENU  hMenu = GetMenu (HWindow);

  ModifyMenu (hMenu, IDM_SAVEAS, MF_BYCOMMAND | MF_STRING,
              IDM_SAVEAS, "Save &As...");

  ModifyMenu (hMenu, CM_EXIT, MF_BYCOMMAND | MF_STRING, CM_EXIT, "E&xit");

  ModifyMenu (hMenu, IDM_UPDATE, MF_BYCOMMAND | MF_STRING, IDM_SAVE, "&Save");
}


/*
    Paint
    -----

    draw the object
*/
void
TWindowServer::Paint (HDC hDC, PAINTSTRUCT _FAR &)
{
  SetViewportOrg (hDC, OBJX, OBJY);
  ((TOLEApp *) GetApplication())->pServer->pDocument->pObject->Draw (hDC);
}



/*
    enum OLEAPPERROR
    ---- -----------

    error codes particular to the OLE app. if we detect an error while
    registering the clipboard formats, initializing the vtable pointers,
    processing the command line, or initializing the server then we set
    "Status" accordingly which will abort the app

    we also override the Error() method accordingly so we can display an
    appropriate error message
*/
enum OLEAPPERROR {olRegClipError = 182, olInitVTBLError = 183};


/*
    CreateServer
    ------------

    process the command line and check for option /Embedding or -Embedding

    then create the OLE server

    there are four scenarios we are concerned with:

    1. Case One: olesrvr.exe
      - embedding = FALSE; create an untitled document

    2. Case two: olesrvr.exe filename
      - embedding = FALSE; create a new document from the file

    3. Case three: olesrvr.exe -Embedding
      - embedding = TRUE; do NOT create or register a document.
                          do NOT show a window until client requests it

    4. Case four: olesrvr.exe -Embedding filename
      - embedding = TRUE; load file, register it (this is the linking case)
                          do NOT show a window until client requests it
*/
void
TOLEApp::CreateServer ()
{
  LPSTR   lpStr = lpCmdLine;
  BOOL    embedded = FALSE;
  LPSTR   szPath = 0;

  //
  // skip any whitespace
  //
  while (*lpStr && *lpStr == ' ')
    lpStr++;

  //
  // check for a '-' or '/'
  //
  if (*lpStr && (*lpStr == '-' || *lpStr == '/')) {
    lpStr++;

    //
    // assume TRUE unless we see otherwise
    //
    embedded = TRUE;

    for (int i = 0; i < sizeof (szEmbedding) - 1; i++)
      if (*lpStr++ != szEmbedding[i]) {
        embedded = FALSE;
        break;
      }
  }

  //
  // skip any whitespace
  //
  while (*lpStr && *lpStr == ' ')
    lpStr++;

  //
  // check for a filename
  //
  if (*lpStr)
    szPath = lpStr;

  //
  // if we are embedded, then we won't display the window until requested
  // to by the library
  //
  if (embedded)
    nCmdShow = SW_HIDE;

  if (szPath)
    new TOLEServer (*this, szPath);

  else
    new TOLEServer (*this, embedded);
}


/*
    RegisterClipboardFormats
    ------------------------

    if you are a mini-server (embedding only) you will need to register
    clipboard formats for "Native" and "OwnerLink"

    if you are a full server (linking and embedding) you will also need to
    register clipboard format "ObjectLink"
*/
BOOL
TOLEApp::RegisterClipboardFormats ()
{
	cfNative = RegisterClipboardFormat ("Native");
	cfOwnerLink = RegisterClipboardFormat ("OwnerLink");
	cfObjectLink = RegisterClipboardFormat ("ObjectLink");

  return cfNative != 0 && cfOwnerLink != 0 && cfObjectLink != 0;
}


/*
    InitInstance
    ------------

    - create the main window
    - create OLE VTBL thunks
    - create clipboard formats
    - parse command line
    - create/register OLE server

    NOTE: we let Windows free all thunks when the application terminates and
          don't do it ourselves
*/
void
TOLEApp::InitInstance ()
{
  MainWindow = new TWindowServer (NULL, szAppName);
  MainWindow = MakeWindow (MainWindow);

  if (!TOLEServer::InitVTBL (hInstance) ||
      !TOLEDocument::InitVTBL (hInstance) ||
      !TOLEObject::InitVTBL (hInstance))
    Status = olInitVTBLError;

  else if (!RegisterClipboardFormats())
    Status = olRegClipError;

  else
    CreateServer();

  //
  // we do this *after* calling CreateServer(), because if we are embedded
  // then we don't want to display the main window until requested to by
  // the server library, and it is CreateServer() who determines that and sets
  // "nCmdShow" accordingly
  //
  if (MainWindow)
    MainWindow->Show (nCmdShow);

  else
    Status = EM_INVALIDMAINWINDOW;
}


/*
    Error
    -----

    trap error messages generated by OLE app, display an error message box
    and terminate the application
*/
void
TOLEApp::Error (int errorCode)
{
  char  *str;

  if (errorCode == olRegClipError)
    str = "Fatal Error: Cannot register ""Native"", ""OwnerLink"", and ""ObjectLink"" clipboard formats";

  else if (errorCode == olInitVTBLError)
    str = "Fatal Error: Cannot create thunks for ""OLESERVER"", ""OLESERVERDOC"", and ""OLEOBJECT"" VTBLs";

  else {
    TApplication::Error (errorCode);
    return;
  }

  MessageBox (0, str, szAppName, MB_OK | MB_ICONSTOP);
  PostAppMessage (GetCurrentTask(), WM_QUIT, 0, 0);
}


/*
    Wait
    ----

    dispatch messages until the given flag is set to TRUE
    one use of this function is to wait until a Release method is called
    after a function has returned OLE_WAIT_FOR_RELEASE.

    PARAMETER: "waitFlag" is a reference to a flag that will be set to TRUE
               when we can return
*/
void
TOLEApp::Wait (BOOL &waitFlag)
{
  MSG   msg;
  BOOL  fMoreMessages = FALSE;

  while (!waitFlag) {
    OleUnblockServer (pServer->lhServer, &fMoreMessages);

    if (!fMoreMessages) {
      //
      // if there are no more messages in the OLE queue, go to system queue
      //
      if (GetMessage (&msg, NULL, NULL, NULL)) {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
      }
    }
  }
}



/*
    WinMain
    -------
*/
int PASCAL
WinMain (HINSTANCE hInstance,
		     HINSTANCE hPrevInstance,
		     LPSTR     lpCmdLine,
		     int       nCmdShow)
{
  TOLEApp  app (szAppName, hInstance, hPrevInstance, lpCmdLine, nCmdShow);

  app.Run ();

  return app.Status;
}
