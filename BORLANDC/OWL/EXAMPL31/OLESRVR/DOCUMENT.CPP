// ObjectWindows - (C) Copyright 1992 by Borland International

/* implementation of class TOLEDocument */

#include <owl.h>

#define SERVERONLY
#include <ole.h>

#include "olesrvr.h"
#include <fstream.h>
#include <CommDlg.h>
#include <string.h>

#define UNNAMED_DOC   "(Untitled)"
OLESERVERDOCVTBL      TOLEDocument::_vtbl;

//
// overloaded stream operators defined in "object.cpp"
//
ostream  &operator<< (ostream &, TOLEObject &);
istream  &operator>> (istream &, TOLEObject &);

/*
    constructor
    -----------

    if "szPath" is NULL then we create an untitled document and default object
    
    the type is "doctypeNew" if "lhDoc" is NULL and "doctypeEmbedded" if
    "lhDoc" is non NULL
    
    if "szPath" is non NULL we create a document of type "doctypeFromFile"
    and initialize it from file "szPath"
    
    if "lhDoc" is NULL then we call OleRegisterServerDoc() otherwise we
    just use "lhDoc" as our registration handle
*/
TOLEDocument::TOLEDocument (TOLEServer &server,
                            LHSERVERDOC lhDoc,
                            LPSTR       szPath,
                            BOOL        dirty)
{
  szName = 0;
  lpvtbl = &_vtbl;
  fRelease = FALSE;
  fDirty = dirty;

  server.pDocument = this;

  //
  // since we only have one object we can create it now...
  //
  pObject = new TOLEObject;

  if (szPath)
    LoadFromFile (szPath);

  else {
    SetDocumentName (UNNAMED_DOC);

    type = lhDoc ? doctypeEmbedded : doctypeNew;
  }

  if (lhDoc != 0)
    this->lhDoc = lhDoc;  // use registration handle we were given

  else
  	OleRegisterServerDoc (server.lhServer, szName, this, (LHSERVERDOC FAR *) &this->lhDoc);
}


/*
    SetDocumentName
    ---------------

    changes instance variable "szName" and changes the window caption
*/
void
TOLEDocument::SetDocumentName (LPSTR newName, BOOL changeCaption)
{
  int   len = lstrlen (newName);

  delete szName;
  szName = new char[len + 1];
  lstrcpy (szName, newName);

  if (changeCaption) {
    char  buf[MAXPATHLENGTH];

    wsprintf (buf, "%s - %s", (LPSTR) szAppName, newName);
    GetApplicationObject()->MainWindow->SetCaption (buf);
  }
}


/*
    LoadFromFile
    ------------

    returns TRUE if successful and FALSE otherwise

    SIDE EFFECTS: if successful sets type to "objtypeFromFile" and sets "szName"
                  to "szPath"
*/
BOOL
TOLEDocument::LoadFromFile (LPSTR szPath)
{
  char      buf[MAXPATHLENGTH];
  TOLEApp  *pApp = (TOLEApp *) GetApplicationObject();

  //
  // in small model if I want to use C++ streams I need to have a near
  // pointer...
  //
  lstrcpy (buf, szPath);

  ifstream  in (buf);

  if (in.fail()) {
      wsprintf (buf, "Cannot open file %s!", szPath);
      MessageBeep (0);
      MessageBox (pApp->MainWindow->HWindow,
                  buf,
                  szAppName, 
                  MB_OK | MB_ICONEXCLAMATION);
    return FALSE;

  } else {
    //
    // read in the signature
    //
    in.read (buf, sizeof (szClassKey) - 1);

    if (strncmp (buf, szClassKey, sizeof (szClassKey) - 1) != 0) {
      wsprintf (buf,
                "File %s is not an \"%s\" file!",
                szPath,
                (LPSTR) szAppName);
      MessageBeep (0);
      MessageBox (pApp->MainWindow->HWindow,
                  buf,
                  szAppName, 
                  MB_OK | MB_ICONEXCLAMATION);

      return FALSE;

    } else {
      in >> *pObject;
      type = doctypeFromFile;
      SetDocumentName (szPath);
    }

    return TRUE;
  }
}


/*
    Reset
    -----

    the only reason that we need this routine is that we re-use the document
    object. if your app doesn't then you would delete the old object and create
    a new one...

    SIDE EFFECTS: sets "fDirty" flag to FALSE and "fRelease" to FALSE

    if "lhDoc" is NULL then call OleRegisterServerDoc()
*/
void
TOLEDocument::Reset (LPSTR szPath)
{
  if (!szPath || !LoadFromFile (szPath)) {
    ((TWindowServer *) GetApplicationObject()->MainWindow)->ShapeChange (objEllipse);

    pObject->native.type = objEllipse;
    pObject->native.version = 1;

    type = doctypeNew;
    SetDocumentName (UNNAMED_DOC);
  }

  if (lhDoc == 0) {
    TOLEApp  *pApp = (TOLEApp *) GetApplicationObject();

  	OleRegisterServerDoc (pApp->pServer->lhServer, szName, this, (LHSERVERDOC FAR *) &lhDoc);
  }

  fDirty = fRelease = FALSE;
}


static
void
Setup (OPENFILENAME *fnStruct)
{
  TOLEApp  *pApp = (TOLEApp *) GetApplicationObject();

  //
  // we want the fully qualified pathname that is returned in "lpstrFile"
  // and not the simple name/extension
  //
  static char simpleName[13]; 

  fnStruct->lStructSize = sizeof (OPENFILENAME);
  fnStruct->nMaxFile = MAXPATHLENGTH;
  fnStruct->Flags = 0;
  fnStruct->hInstance = pApp->hInstance;
  fnStruct->hwndOwner = pApp->MainWindow->HWindow;
  fnStruct->lCustData = 0;
  fnStruct->lpfnHook = 0;
  fnStruct->lpstrCustomFilter = 0;
  fnStruct->lpstrDefExt = szFileExt;
  fnStruct->lpstrFilter = "OWL OLE Server (*." szFileExt ")\0*." szFileExt "\0" ;
  fnStruct->lpstrFileTitle = simpleName;
  fnStruct->lpstrInitialDir = 0;
  fnStruct->lpstrTitle = 0;  // use default
  fnStruct->lpTemplateName = 0;
  fnStruct->nFilterIndex = 1;
  fnStruct->nFileOffset = 0;
  fnStruct->nFileExtension = 0;
  fnStruct->nMaxCustFilter = 0;
}


/*
    PromptForOpenFileName
    ---------------------
*/
BOOL
TOLEDocument::PromptForOpenFileName (char *path)
{
  OPENFILENAME  fnStruct;

  Setup (&fnStruct);

  fnStruct.Flags |= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
  wsprintf (path, "*.%s", (LPSTR) szFileExt);
  fnStruct.lpstrFile = path;

  return GetOpenFileName (&fnStruct);
}


/*
    SaveAs
    ------

    calls the common Windows dialog function to prompt the user for the
    filename to use
*/
void
TOLEDocument::SaveAs ()
{
  char          path[MAXPATHLENGTH];  // result of GetSaveFileName()
  OPENFILENAME  fnStruct;

  Setup (&fnStruct);
  fnStruct.Flags |= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
  wsprintf (path, "*.%s", (LPSTR) szFileExt);
  fnStruct.lpstrFile = path;

  if (GetSaveFileName (&fnStruct)) {
    type = doctypeFromFile;
    SetDocumentName (path);  // we must do this BEFORE we call SaveDoc()
    SaveDoc();

    //
    // now inform the server library that we have renamed the document
    //
    OleRenameServerDoc (lhDoc, szName);
  }
}


/*
    SaveDoc
    -------

    save the document to file "szName" and mark the document as not "dirty"
*/
void
TOLEDocument::SaveDoc ()
{
  if (type == doctypeNew)
    SaveAs();

  else {
    ofstream  out (szName);

    out << szClassKey << " ";  // signature
    out << *pObject;
    fDirty = FALSE;
  }
}


/*
    callback Close
    -------- -----

    we have been requested to close the document because the client that
    contains a link (embedding or linking) to that document has shut down

    this method is always called *before* the document's "Release" method
    is called

    WHAT TO DO:
      - call OleRevokeServerDoc() and *don't* free any resources until
        "Release" is called
      - return the value of OleRevokeServerDoc()

    NOTE: since TOLEDocument is a subclass of OLESERVERDOC we can just cast
          a LPOLESERVERDOC to a (TOLEDocument FAR *)
*/
OLESTATUS FAR PASCAL _export
TOLEDocument::Close (LPOLESERVERDOC lpOleDoc)
{
  return OleRevokeServerDoc (((TOLEDocument FAR *) lpOleDoc)->lhDoc);
}


/*
    callback GetObjet
    -------- --------

    server library calls this method whenever a client application creates
    an object using a function like OleCreate()

    if "lpszObjName" is NULL, that means we are being called for an embedded
    object after the server was sent "Create", "Edit", or "CreateFromTemplate"
    and the server library requests the entire document

    if "lpszObjName" isn't NULL then the server has already received a "Open"
    message to activate the linked object

    WHAT TO DO:
      - allocate a TOLEObject if "lpszObjName" is NULL, or look up "lpszObjName"
        in the list of objects if it isn't NULL
      - store the pointer to the TOLEObject in "lplpOleObject"
      - store "lpOleClient" so we can send notifications to the client
        (used for linked objects)
      - return OLE_OK if successful, OLE_ERROR_NAME if "lpszObjName" isn't
        recognized, or OLE_ERROR_MEMORY if the object could not be allocated

    NOTE:
      - we only have one object and it is created when the document is
        created. Therefore, we don't actually create anything here...

      - "lpOleClient" resides in the rever library and is used on behalf of
        a client application
*/
OLESTATUS FAR PASCAL _export
TOLEDocument::GetObject (LPOLESERVERDOC lpOleDoc,
	                       LPSTR          lpszObjName,
	                       LPOLEOBJECT    FAR *lplpOleObj,
	                       LPOLECLIENT    lpOleClient)
{
  TOLEDocument  *pDoc = (TOLEDocument *) lpOleDoc;

  //
  // in either case (whether "lpszObjName" is null or not) we just return
  // the object associated with the document
  //
  *lplpOleObj = pDoc->pObject;

  //
  // if "lpszObjName" isn't NULL then we associate client "lpOleClient" with
  // it...
  //
  // NOTE: we only have one object. if you have multiple objects then you would
  //       have to search your objects to find the one that matched "lpszObjName"
  //
  if (lpszObjName)
    pDoc->pObject->AddClientLink (lpOleClient);

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
TOLEDocument::Execute (LPOLESERVERDOC /* lpOleDoc */,
	                     HANDLE         /* hCommands */)
{
	return OLE_ERROR_COMMAND;
}


/*
    callback Release
    -------- -------

    the server library calls this routine when all conversations to the
    object have been closed. at this point the server has called either
    OleRevokeServerDoc() or OleRevokeServer()

    there will be no more calls to the document's methods. it is okay to
    free the document's objects, but *not* the TOLEDocument yet...

    WHAT TO DO:
      - free the document's objects and resources (e.g. atoms) but *not* the
        document itself
      - set a flag to indicate that "Release" has been called
      - return OLE_OK if successful, OLE_ERROR_GENERIC otherwise

    NOTE: since we only have one document and one object within the
          documeny we don't NOT delete the object here

          however, you might want to...
*/
OLESTATUS FAR PASCAL _export
TOLEDocument::Release (LPOLESERVERDOC lpOleDoc)
{
  ((TOLEDocument *) lpOleDoc)->fRelease = TRUE;
	return OLE_OK;
}


/*
    callback Save
    -------- ----

    this method is only used when the server is editing a linked object:

    the client application is closing and the user has requested saving the
    client document which contains a linked object

    WHAT TO DO:
      - save the document to the filename which was passed in when the document
        was opened for linking
      - return OLE_OK if successful, OLE_ERROR_GENERIC otherwise
*/
OLESTATUS FAR PASCAL _export
TOLEDocument::Save (LPOLESERVERDOC lpOleDoc)
{
  TOLEDocument  *doc = (TOLEDocument *) lpOleDoc;

  if (doc->type != doctypeFromFile)
    return OLE_ERROR_GENERIC;

  else {
    doc->SaveDoc();
    return OLE_OK;
  }
}


/*
    callback SetColorScheme
    -------- --------------
*/
OLESTATUS FAR PASCAL _export
TOLEDocument::SetColorScheme (LPOLESERVERDOC /* lpOleDoc */,
	                            LPLOGPALETTE   /* lpPal */)
{
	return OLE_ERROR_GENERIC;
}


/*
    callback DocSetDimensions
    -------- ----------------

    the client is informing  us how big the object should be

    "lpRect" is in MM_HIMETRIC units (all OLE libraries express the size of
     every object in MM_HIMETRIC)
*/
OLESTATUS FAR PASCAL _export
TOLEDocument::SetDocDimensions (LPOLESERVERDOC /* lpOleDoc */,
	                              LPRECT         /* lpRect */)
{
	return OLE_OK;
}


/*
    callback SetHostNames
    -------- ------------

    the server library is calling to provide the server with the name of the
    client's document and the name of the object in the client application

    these names should be used to make the necessary window title bar and
    menu changes

    this method is only called for embedded objects because linked objects
    display their filename in the title bar

    WHAT IT DOES:
      - change the title bar and File menu
      - store the object and client names for later use
      - return OLE_OK is successful, OLE_ERROR_GENERIC otherwise

    PARAMETERS:
      - "lpClient" is the name of the client application document
      - "lpDoc" is the name of the object in the client application
*/
OLESTATUS FAR PASCAL _export
TOLEDocument::SetHostNames (LPOLESERVERDOC lpOleDoc,
	                          LPSTR          lpClient,
	                          LPSTR          lpDoc)
{
  ((TWindowServer *) GetApplicationObject()->MainWindow)->UpdateFileMenu (lpDoc);

  //
  // store the document name, but don't update the title bar we will do that
  // below
  //
  ((TOLEDocument *) lpOleDoc)->SetDocumentName (lpDoc, FALSE);

  //
  // set the caption to be <App Name> - <Object Name> in <Client App Document>
  //
  char  buf[128];

  wsprintf (buf, "%s - %s in %s", (LPSTR) szAppName, lpDoc, lpClient);
  GetApplicationObject()->MainWindow->SetCaption (buf);
	return OLE_OK;
}



typedef OLESTATUS FAR PASCAL _export (FAR *LPDOCCLOSE)(LPOLESERVERDOC);
typedef OLESTATUS FAR PASCAL _export (FAR *LPDOCGETOBJECT)(LPOLESERVERDOC,const char far *,LPOLEOBJECT FAR *,LPOLECLIENT);
typedef OLESTATUS FAR PASCAL _export (FAR *LPDOCEXEC)(LPOLESERVERDOC,HANDLE);
typedef OLESTATUS FAR PASCAL _export (FAR *LPDOCRELEASE)(LPOLESERVERDOC);
typedef OLESTATUS FAR PASCAL _export (FAR *LPDOCSAVE)(LPOLESERVERDOC);
typedef OLESTATUS FAR PASCAL _export (FAR *LPDOCSETCOLORSCHM)(LPOLESERVERDOC,const LOGPALETTE far *);
typedef OLESTATUS FAR PASCAL _export (FAR *LPDOCSETDIMNSN)(LPOLESERVERDOC,const RECT far *);
typedef OLESTATUS FAR PASCAL _export (FAR *LPDOCSETHOSTNAMES)(LPOLESERVERDOC,const char far *,const char far *);


/*
    InitVTBL
    --------

    create thunks for OLESERVERDOC method callback tables
*/
BOOL
TOLEDocument::InitVTBL (HINSTANCE hInstance)
{
	_vtbl.Close = (LPDOCCLOSE) MakeProcInstance ((FARPROC) Close, hInstance);
	_vtbl.GetObject = (LPDOCGETOBJECT) MakeProcInstance ((FARPROC) GetObject, hInstance);
	_vtbl.Execute = (LPDOCEXEC) MakeProcInstance ((FARPROC) Execute, hInstance);
	_vtbl.Release = (LPDOCRELEASE) MakeProcInstance ((FARPROC) Release, hInstance);
	_vtbl.Save = (LPDOCSAVE) MakeProcInstance ((FARPROC) Save, hInstance);
	_vtbl.SetColorScheme = (LPDOCSETCOLORSCHM) MakeProcInstance ((FARPROC) SetColorScheme, hInstance);
	_vtbl.SetDocDimensions = (LPDOCSETDIMNSN) MakeProcInstance ((FARPROC) SetDocDimensions, hInstance);
	_vtbl.SetHostNames = (LPDOCSETHOSTNAMES) MakeProcInstance ((FARPROC) SetHostNames, hInstance);

  return _vtbl.Close != NULL &&
         _vtbl.GetObject != NULL &&
         _vtbl.Execute != NULL &&
         _vtbl.Release != NULL &&
         _vtbl.Save != NULL &&
         _vtbl.SetColorScheme != NULL &&
         _vtbl.SetDocDimensions != NULL &&
         _vtbl.SetHostNames != NULL;
}

