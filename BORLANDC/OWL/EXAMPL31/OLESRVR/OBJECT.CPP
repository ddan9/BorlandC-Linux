// ObjectWindows - (C) Copyright 1992 by Borland International

/* implementation of class TOLEObject */

#include <owl.h>

#define SERVERONLY
#include <ole.h>

#include <ctype.h>
#include "olesrvr.h"
#include <string.h>
#include <fstream.h>

OLEOBJECTVTBL  TOLEObject::_vtbl;


/*
    constructor
    -----------
*/
TOLEObject::TOLEObject ()
{
  lpvtbl = &_vtbl;
  native.type = objEllipse;
  native.version = 1;
  lpClients[0] = 0;
  fRelease = FALSE;
}


ostream &operator<< (ostream &out, TOLEObject &obj)
{
  out << int (obj.native.type) << " ";
  out << obj.native.version;
  return out;
}


istream  &operator>> (istream &in, TOLEObject &obj)
{
  int  newType;

  in >> newType;
  ((TWindowServer *) GetApplicationObject()->MainWindow)->ShapeChange ((NATIVETYPE) newType);
  obj.native.type = (NATIVETYPE) newType;
  in >> obj.native.version;
  return in;
}


/*
    SetType
    -------

    sets the "type" instance variable and calls ObjectChanged()
*/
void
TOLEObject::SetType (NATIVETYPE type)
{
  native.type = type;

  ObjectChanged();
}


/*
    ObjectChanged
    -------------

    send each of the clients we are linked to an OLE_CHANGED message
*/
void
TOLEObject::ObjectChanged ()
{
  TOLEDocument  *pDoc = ((TOLEApp *) GetApplicationObject())->pServer->pDocument;

  //
  // call the object through its callback function
  //
  if (lpClients[0])
    for (int i = 0; lpClients[i] != 0; i++)
      lpClients[i]->lpvtbl->CallBack (lpClients[i], OLE_CHANGED, this);

  //
  // mark the document as changed
  //
  pDoc->fDirty = TRUE;
}


/*
    AddClientLink
    -------------
*/
void
TOLEObject::AddClientLink (LPOLECLIENT lpOleClient)
{
  //
  // we always append clients to the end of the list
  //
  // find the next slot
  //
  for (int i = 0; i < MAXLINKS; i++)
    if (lpClients[i] == 0) {
      lpClients[i] = lpOleClient;
      lpClients[i + 1] = 0;  // NULL terminator
      return;
    }
}


/*
    Draw
    ----

    draw the type specified by "type" using the device context that is passed
    in
*/
void
TOLEObject::Draw (HDC hDC)
{
  HANDLE  hOldBrush = SelectObject (hDC, CreateSolidBrush (RGB (255, 0, 0)));
  HANDLE  hOldPen = SelectObject (hDC, GetStockObject (NULL_PEN));

	switch (native.type) {
		case objEllipse:
      Ellipse (hDC, 0, 0, OBJWIDTH, OBJHEIGHT);
			break;

		case objRect:
			Rectangle (hDC, 0, 0, OBJWIDTH, OBJHEIGHT);
			break;

		case objTriangle:
      int    xCenter = OBJWIDTH / 2;
      POINT  pts[] = {{xCenter, 0},
                      {0, OBJHEIGHT - 1},
                      {OBJWIDTH - 1, OBJHEIGHT - 1},
                      {xCenter, 0}};

      Polygon (hDC, pts, 4);
			break;
		}

  DeleteObject (SelectObject (hDC, hOldBrush));
  SelectObject (hDC, hOldPen);
}


/*
    GetNativeData
    -------------

    returns a global memory handle that contains the native data for the
    receiver

    this handle can be used to set the Native clipboard data format
*/
HANDLE
TOLEObject::GetNativeData ()
{
  HANDLE     hData;
  LPNATIVE   lpData;

  hData = GlobalAlloc (GMEM_DDESHARE, sizeof (native));

  if (hData) {
    lpData = (LPNATIVE) GlobalLock (hData);
    *lpData = native;
    GlobalUnlock (hData);
  }

  return hData;
}


/*
    GetLinkData
    -----------

    returns a global memory handle that contains three fields:

    - class name
    - document name (typically a fully qualified path name that identifies
      the file containing the document)
    - item name (uniquely identifies the part of the document that is defined
      as an object)

    the class name and document name are null terminated, and the item name
    has two terminating null characters, e.g. CNAME\0DNAME\0INAME\0\0

    NOTE: item names are assigned by the server. since we have only 1 object
          per document, we always use the same name ("1"). most applications
          would use a different strategy, e.g. "Object1", "Object2", ...

    since "ObjectLink" and "OwnerLink" formats contain the same information
    the handle that is returned can be used for both clipboard formats
*/
HANDLE
TOLEObject::GetLinkData ()
{
  HANDLE         hData;
  LPSTR          lpData;
  TOLEDocument  *pDoc = ((TOLEApp *) GetApplicationObject())->pServer->pDocument;
  int            docNameLen = strlen (pDoc->szName);
  const int      classKeyLen = sizeof (szClassKey) - 1;        
  int            len =  classKeyLen + docNameLen + strlen ("1") + 4;

  hData = GlobalAlloc (GMEM_DDESHARE, len);

  if (hData) {
    lpData = (LPSTR) GlobalLock (hData);
  
    //
    // write class name
    //
    lstrcpy (lpData, szClassKey);
    lpData += classKeyLen + 1;
  
    //
    // write document name
    //
    lstrcpy (lpData, pDoc->szName);
    lpData += docNameLen + 1;
  
    //
    // write item name (we always write "1")
    //
    *lpData++ = '1';
    *lpData++ = '\0';
  
    //
    // write extra null terminator
    //
    *lpData = '\0';
  
    GlobalUnlock (hData);
  }

  return hData;
}


/*
    SizeToHiMetric
    --------------

    convert a width and height from device units to MM_HIMETRIC units
    which are required by the OLE libraries
*/
static
void
SizeToHiMetric (int &width, int &height)
{
  HDC         hDC = GetDC (0);  // screen
  int         dpiX = GetDeviceCaps (hDC, LOGPIXELSX);
  int         dpiY = GetDeviceCaps (hDC, LOGPIXELSY);
  const long  HiMetricPerInch = 2540;

  width = int (width * HiMetricPerInch / dpiX);
  height = int (height * HiMetricPerInch / dpiY);

  ReleaseDC (0, hDC);
}


/*
    GetMetafilePicture
    ------------------
*/
HANDLE
TOLEObject::GetMetafilePicture ()
{
  LPMETAFILEPICT  lpPict;
  HANDLE          hPict;
  HMETAFILE       hMF;
  HDC             hDC = CreateMetaFile (NULL);
  int             width = 100, height = 100;

  //
  // draw the object into the metafile
  //
  SetWindowOrg (hDC, 0, 0);
  SetWindowExt (hDC, width, height);
  Draw (hDC);

  //
  // get the handle to the metafile.
  //
  hMF = CloseMetaFile (hDC);

  //
  // allocate the metafile picture
  //
  hPict = GlobalAlloc (GMEM_DDESHARE, sizeof (METAFILEPICT));

  if (hPict) {
    SizeToHiMetric (width, height);
    lpPict = (LPMETAFILEPICT) GlobalLock (hPict);
  
    lpPict->mm   = MM_ANISOTROPIC;
    lpPict->hMF  = hMF;
    lpPict->xExt = width;
    lpPict->yExt = height;

    GlobalUnlock (hPict);
  }

  return hPict;
}


/*
    GetBitmapData
    -------------
*/
HBITMAP
TOLEObject::GetBitmapData ()
{
  HWND     hWnd = GetApplicationObject()->MainWindow->HWindow;
  HDC      hDC = GetDC (hWnd);
  HDC      hMemoryDC = CreateCompatibleDC (hDC);
  HBITMAP  hBitmap = CreateCompatibleBitmap (hDC, 100, 100);
  HBITMAP  hOldBitmap = (HBITMAP) SelectObject (hMemoryDC, hBitmap);
  int      width = 100, height = 100;

  ReleaseDC (hWnd, hDC);
  PatBlt (hMemoryDC, 0, 0, width, height, WHITENESS);
  Draw (hMemoryDC);
  SelectObject (hMemoryDC, hOldBitmap);
  DeleteDC (hMemoryDC);

  //
  // convert the width and height to MM_HIMETRIC (all OLE libraries express
  // the size of every object in MM_HIMETRIC)
  //
  SizeToHiMetric (width, height);

  //
  // SetBitmapDimension() wants the width and height in .1 millimeter
  // units, so we must divide by 10...
  //
  SetBitmapDimension (hBitmap, width / 10, height / 10);

  return hBitmap;
}


/*
    callback DoVerb
    -------- ------

    client application has called OleActivate() on an embedded object and
    requests an action on the object

    the action is specified by the verb identifier "wVerb"

    our server only understands EDIT and PLAY:
      - all we do for PLAY is beep

      - for EDIT we bring up the server and let the user edit the specified
        object

    PARAMETERS:
      - "wVerb" is the index to the verb to execute
      - "fShow" indicates if the server should show the object or remain in
        its current state
      - "fFocus" indicates if the server should take the focus

    WHAT TO DO:
      - for PLAY verb, a server doesn't usually show its window or affect the
        focus
      - for EDIT verb, show the server's window and object if "fShow" and take
        the focus is "fFocus"
      - return OLE_OK if successful, OLE_ERROR_DOVERB otherwise
*/
OLESTATUS FAR PASCAL _export
TOLEObject::DoVerb (LPOLEOBJECT lpOleObject,
	                  unsigned    int wVerb,
	                  BOOL        fShow,
	                  BOOL        fTakeFocus)
{
  switch (wVerb) {
    case verbEdit:
      //
      // the easiest way to show the server's window is to send the object a
      // "Show" message
      //
      return fShow ? lpOleObject->lpvtbl->Show (lpOleObject, fTakeFocus) : OLE_OK;

    case verbPlay:
      for (int i = 0; i < 10; i++)
        MessageBeep (0);

      return OLE_OK;

    default:
      return OLE_ERROR_DOVERB;
  }
}


/*
    callback EnumFormats
    -------- -----------

    client has requested that we enumerate all clipboard formats that we
    support for object "lpOleObject"

    the server library will make multiple calls until we return the format
    that the server library is looking for

    PARAMETERS:
      - "cfFormat" is the last format returned by this method. if it is 0 then
        this is the first call to the method for this series

    we terminate the query by returning NULL

    NOTE: we *must* return the formats in the same order as the order that
          data is placed on the clipboard!
*/
OLECLIPFORMAT FAR PASCAL _export
TOLEObject::EnumFormats (LPOLEOBJECT   /* lpOleObject */,
	                       OLECLIPFORMAT cfFormat)
{
  TOLEApp  *pApp = (TOLEApp *) GetApplicationObject();

  //
  // if "cfFormat" is 0 that indicates the client wants us to return the
  // first format
  //
  if (cfFormat == 0)
    return pApp->cfNative;

  else if (cfFormat == pApp->cfNative)
    return pApp->cfOwnerLink;

  else if (cfFormat == pApp->cfOwnerLink)
    return CF_METAFILEPICT;

  else if (cfFormat == CF_METAFILEPICT)
    return CF_BITMAP;

  else
    return NULL;
}


/*
    callback GetData
    -------- -------

    we are requested to supply data for the object in a specific format,
    such as Native or CF_METAFILEPICT

    in general you should handle the same data formats that you put on the
    clipboard when the object was embedded/linked

    these should be the same formats that are returned by method EnumFormats()

    requests foro GetData occur at any time that the client needs to display
    an object or when the data must be written to a client file
*/
OLESTATUS FAR PASCAL _export
TOLEObject::GetData (LPOLEOBJECT lpOleObject,
	                   WORD        cfFormat,
	                   LPHANDLE    lpHandle)
{
  TOLEApp     *pApp = (TOLEApp *) GetApplicationObject();
  TOLEObject  *pObject = (TOLEObject *) lpOleObject;  // just a cast

  if (cfFormat == pApp->cfNative) {
    *lpHandle = pObject->GetNativeData();

    return *lpHandle ? OLE_OK : OLE_ERROR_MEMORY;
  }

  if (cfFormat == pApp->cfOwnerLink) {
    *lpHandle = pObject->GetLinkData();

    return *lpHandle ? OLE_OK : OLE_ERROR_MEMORY;
  }

  if (cfFormat == CF_BITMAP) {
    *lpHandle = pObject->GetBitmapData();

    return *lpHandle ? OLE_OK : OLE_ERROR_MEMORY;
  }

  if (cfFormat == CF_METAFILEPICT) {
    *lpHandle = pObject->GetMetafilePicture();

    return *lpHandle ? OLE_OK : OLE_ERROR_MEMORY;
  }

  return OLE_ERROR_FORMAT;
}


/*
    callback QueryProtocol
    -------- -------------

    server library is trying to determine which protocols we support

    "lpszProtocol" will either be "StdFileEditing" or "StdExecute"

    if we don't support the protocol then we should return NULL

    since we don't support "StdFileExecute" we return NULL in that case
*/    
LPVOID FAR PASCAL _export
TOLEObject::QueryProtocol (LPOLEOBJECT lpOleObject,
	                         LPSTR       lpszProtocol)
{
	return lstrcmp (lpszProtocol, "StdFileEditing") ? NULL : lpOleObject;
}


/*
    callback Release
    -------- -------

    this method gets called when the library wants to inform us that we have
    no more clients connected to the object

    it is initiated after the client calls OleDelete() or the server calls
    OleRevokeServer(), OleRevokeServerDoc(), or OleRevokeObject()

    this is the last time that the receiving object will be called, so all
    resources for the object can be free'd, but we MUST not delete the object
    itself...

    WHAT TO DO:
      - free resourcess associated with the object
      - set a flag to indicate "Release" has been called
      - NULL out any saved LPOLECLIENT handles saved in the object
      - return OLE_OK if successful, OLE_ERROR_GENERIC otherwise
*/
OLESTATUS FAR PASCAL _export
TOLEObject::Release (LPOLEOBJECT lpOleObject)
{
  TOLEObject  *pObject = (TOLEObject *) lpOleObject;

  pObject->lpClients[0] = 0;
  pObject->fRelease = TRUE;
	return OLE_OK;
}


/*
    callback SetBounds
    -------- ---------
*/
OLESTATUS FAR PASCAL _export
TOLEObject::SetBounds (LPOLEOBJECT /* lpoleobj */,
	                     LPRECT      /* lprect */)
{
	return OLE_ERROR_GENERIC;
}


/*
    callback SetColorScheme
    -------- --------------
*/
OLESTATUS FAR PASCAL _export
TOLEObject::SetColorScheme (LPOLEOBJECT  /* lpOleObject */,
	                          LPLOGPALETTE /* lpPal */)
{
	return OLE_ERROR_GENERIC;
}


/*
    callback SetData
    -------- -------

    this routine gets called to provide the server with the data for an
    object that is embedded in a client

    this routine gets called after the server has received an "Edit" message

    this method is always called before "DoVerb" and "Show"

    WHAT TO DO:
      - if the data format isn't Native, return OLE_ERROR_FORMAT
      - lock down the memory to get a pointer to the data, returning
        OLE_ERROR_MEMORY if GlobalLock() returns NULL
      - copy the data to "lpOleObject"
      - unlock the memory and call GlobalFree() on the handle (you are
        responsible for the memory!)
      - return OLE_OK
*/
OLESTATUS FAR PASCAL _export
TOLEObject::SetData (LPOLEOBJECT   lpOleObject,
	                   OLECLIPFORMAT cfFormat,
	                   HANDLE        hData)
{
  TOLEApp  *pApp = (TOLEApp *) GetApplicationObject();

  if (cfFormat != pApp->cfNative)
    return OLE_ERROR_FORMAT;  // data isn't in Native format

  else {
    LPNATIVE  lpData = (LPNATIVE) GlobalLock (hData);

    if (lpData == NULL)
      return OLE_ERROR_MEMORY;

    else {
      TOLEObject  *pObject = (TOLEObject *) lpOleObject;

      ((TWindowServer *) pApp->MainWindow)->ShapeChange (lpData->type);
      pObject->native = *lpData;
      pApp->pServer->pDocument->fDirty = FALSE;

      GlobalUnlock (hData);
      GlobalFree (hData);    
      return OLE_OK;
    }
  }
}


/*
    callback SetTargetDevice
    -------- ---------------
*/
OLESTATUS FAR PASCAL _export
TOLEObject::SetTargetDevice (LPOLEOBJECT /* lpOleObject */,
	                           HANDLE      /* hData */)
{
	return OLE_ERROR_GENERIC;
}


/*
    callback Show
    -------- ----

    this method gets called when we should make the object visible by
    making the server window visible and possibly scroling the object into
    view

    if the object is selectable, select it as well

    PARAMETERS:
      - "fTakeFocus" indicates whether the server should set focus to itself

    WHAT TO DO:
      - show the window(s) if not visible
      - scroll "lpOleObject" into view and select it if possible
      - if "fTakeFocus" is TRUE, call SetFocus() with the main window handle
      - return OLE_OK if successful, OLE_ERROR_GENERIC otherwise
*/
OLESTATUS FAR PASCAL _export
TOLEObject::Show (LPOLEOBJECT /* lpOleObject */,
	                BOOL        fTakeFocus)
{
  TOLEApp  *pApp = (TOLEApp *) GetApplicationObject();

  //
  // in our case all we need to do is request that the window is showing
  //
  pApp->MainWindow->Show (SW_SHOWNORMAL);

  if (fTakeFocus)
    SetFocus (pApp->MainWindow->HWindow);

  return OLE_OK;
}



typedef OLESTATUS FAR PASCAL _export (FAR *LPOBJDOVERB)(LPOLEOBJECT,unsigned int,BOOL,BOOL);
typedef OLECLIPFORMAT FAR PASCAL _export (FAR *LPOBJENUMFORMATS)(LPOLEOBJECT,OLECLIPFORMAT);
typedef OLESTATUS FAR PASCAL _export (FAR *LPOBJGETDATA)(LPOLEOBJECT,WORD,LPHANDLE);
typedef LPVOID 		FAR PASCAL _export (FAR *LPOBJQUERYPROTOCOL)(LPOLEOBJECT,const char far *);
typedef OLESTATUS FAR PASCAL _export (FAR *LPOBJRELEASE)(LPOLEOBJECT);
typedef OLESTATUS FAR PASCAL _export (FAR *LPOBJSETBOUNDS)(LPOLEOBJECT,const RECT far *);
typedef OLESTATUS FAR PASCAL _export (FAR *LPOBJSETCOLORSCHM)(LPOLEOBJECT,const LOGPALETTE far *);
typedef OLESTATUS FAR PASCAL _export (FAR *LPOBJSETDATA)(LPOLEOBJECT,OLECLIPFORMAT,HANDLE);
typedef OLESTATUS FAR PASCAL _export (FAR *LPOBJSETTARGETDEVICE)(LPOLEOBJECT,HANDLE);
typedef OLESTATUS FAR PASCAL _export (FAR *LPOBJSHOW)(LPOLEOBJECT,BOOL);


/*
    InitVTBL
    --------

    create thunks for OLEOBJECT method callback tables
*/
BOOL
TOLEObject::InitVTBL (HINSTANCE hInstance)
{
	_vtbl.DoVerb = (LPOBJDOVERB) MakeProcInstance ((FARPROC) DoVerb, hInstance);
	_vtbl.EnumFormats = (LPOBJENUMFORMATS) MakeProcInstance ((FARPROC) EnumFormats, hInstance);
	_vtbl.GetData = (LPOBJGETDATA) MakeProcInstance ((FARPROC) GetData, hInstance);
	_vtbl.QueryProtocol = (LPOBJQUERYPROTOCOL) MakeProcInstance ((FARPROC) QueryProtocol, hInstance);
	_vtbl.Release = (LPOBJRELEASE) MakeProcInstance ((FARPROC) Release, hInstance);
	_vtbl.SetBounds = (LPOBJSETBOUNDS) MakeProcInstance ((FARPROC) SetBounds, hInstance);
	_vtbl.SetColorScheme = (LPOBJSETCOLORSCHM) MakeProcInstance ((FARPROC) SetColorScheme, hInstance);
	_vtbl.SetData = (LPOBJSETDATA) MakeProcInstance ((FARPROC) SetData, hInstance);
	_vtbl.SetTargetDevice = (LPOBJSETTARGETDEVICE) MakeProcInstance ((FARPROC) SetTargetDevice, hInstance);
	_vtbl.Show = (LPOBJSHOW) MakeProcInstance ((FARPROC) Show, hInstance);

  return _vtbl.DoVerb != NULL &&
         _vtbl.EnumFormats != NULL &&
         _vtbl.GetData != NULL &&
         _vtbl.QueryProtocol != NULL &&
         _vtbl.Release != NULL &&
         _vtbl.SetBounds != NULL &&
         _vtbl.SetColorScheme != NULL &&
         _vtbl.SetData != NULL &&
         _vtbl.SetTargetDevice != NULL &&
         _vtbl.Show != NULL;
}

