// ObjectWindows - (C) Copyright 1992 by Borland International

/* application "OWL OLE Server" which demonstrates a multiple instances (i.e.
   non-MDI) OLE server supporting linking and embedding */

//
// "File" menu items
//
#define IDM_NEW         100
#define IDM_OPEN        101
#define IDM_SAVE        102
#define IDM_SAVEAS      103
#define IDM_UPDATE      104

//
// "Edit" menu items
//
#define IDM_COPY        200

//
// "Shape" menu items
//
#define IDM_ELLIPSE     300
#define IDM_RECTANGLE   301
#define IDM_TRIANGLE    302

//
// "Help" menu items
//
#define IDM_ABOUT       400

#define szAppName       "OWL OLE Server"
#define szClassKey      "OWLOLEServer"
#define szClassValue    "OWL OLE Object"
#define szExeName       "olesrvr"
#define szFileExt       "oos"

const int  MAXPATHLENGTH = 256;
const int  MAXLINKS = 10;

const int  OBJX = 75;
const int  OBJY = 50;
const int  OBJWIDTH = 100;
const int  OBJHEIGHT = 100;

_CLASSDEF (TOLEServer)
_CLASSDEF (TOLEApp)


/*
    enum DOCTYPE
    ---- -------
    document type
*/
enum DOCTYPE {doctypeNew,        // document is untitled
              doctypeFromFile,   // document exists in a file and may be linked
              doctypeEmbedded};  // document is an embedded document


/*
    typedef VERSION
    ------- -------

    server applications should store version numbers in their Native data
    formats

    this way a client application may embed data from one version of a server
    and later request a newer version to edit that data
*/
typedef int VERSION;


/*
    enum NATIVETYPE
    ---- ----------

    native data type
*/
enum NATIVETYPE {objEllipse = IDM_ELLIPSE,
                 objRect = IDM_RECTANGLE,
                 objTriangle = IDM_TRIANGLE};


/*
    struct NATIVE
    ------ ------

    native data format
*/
struct NATIVE {
  NATIVETYPE  type;
  VERSION     version;
};

typedef struct NATIVE  FAR *LPNATIVE;


/*
    enum VERB
    ---- ----

    the types of actions that the server can perform on an object
*/
enum VERB {verbEdit, verbPlay};


/*
    enum FILEIOSTATUS
    ---- ------------

    used when prompting the user to save changes before performing a file
    I/O command

    if the user chooses "Cancel" then "fiCancel" is returned; otherwise
    "fiExecute" is returned
*/
enum FILEIOSTATUS {fiCancel, fiExecute};


/*
    class TOLEObject
    ----- ----------

    although we have embedded the native data in the ole object, you might
    not want to do this.

    rather than integrate OLE with your app you should treat OLE as a protocol
    that sits on top of your app and allows other applications access to your
    server's data.

    instead of embedding the data in the OLE object have the OLE object contain
    a pointer to the native data.
*/
_CLASSDEF (TOLEObject)

class TOLEObject : public OLEOBJECT {
  public:
    NATIVE       native;
    BOOL         fRelease;  // TRUE if Release method has been called
    LPOLECLIENT  lpClients[MAXLINKS + 1];  // null terminated list of client(s)
                                           // we are linked to

  TOLEObject ();

    NATIVETYPE    GetType () {return native.type;}

    //
    // sets instance variable "type" and calls ObjectChanged()
    //
    void          SetType (NATIVETYPE);

    //
    // marks the document as "dirty" and sends each of the clients the receiver
    // is linked to an OLE_CHANGED message
    //
    void          ObjectChanged ();

    //
    // add "lpOleClient" to the list of clients that should be notified when
    // the receivber changes
    //
    void          AddClientLink (LPOLECLIENT lpOleClient);

    void          Draw (HDC);

    //
    // routines to build the various clipboard formats that are required for
    // an OLE server
    //
    // your routine might provide routines for additional formats such as TEXT,
    // RTF, and DIB
    //
    HANDLE        GetNativeData ();
    HANDLE        GetLinkData ();
    HBITMAP       GetBitmapData ();
    HANDLE        GetMetafilePicture ();

    //
    // vtable initialization
    //
    static BOOL   InitVTBL (HINSTANCE);

    //
    // vtable entries
    //
    static LPVOID 	     FAR PASCAL _export QueryProtocol (LPOLEOBJECT, LPSTR);
    static OLESTATUS     FAR PASCAL _export Release (LPOLEOBJECT);
    static OLESTATUS     FAR PASCAL _export Show (LPOLEOBJECT, BOOL);
    static OLESTATUS     FAR PASCAL _export DoVerb (LPOLEOBJECT, UINT, BOOL, BOOL);
    static OLESTATUS     FAR PASCAL _export GetData (LPOLEOBJECT, WORD, LPHANDLE);
    static OLESTATUS     FAR PASCAL _export SetData (LPOLEOBJECT, OLECLIPFORMAT, HANDLE);
    static OLESTATUS     FAR PASCAL _export SetTargetDevice (LPOLEOBJECT, HANDLE);
    static OLESTATUS     FAR PASCAL _export SetBounds (LPOLEOBJECT, LPRECT);
    static OLECLIPFORMAT FAR PASCAL _export EnumFormats (LPOLEOBJECT, OLECLIPFORMAT);
    static OLESTATUS     FAR PASCAL _export SetColorScheme (LPOLEOBJECT, LPLOGPALETTE);

  private:
    static OLEOBJECTVTBL  _vtbl;
};


/*
    class TOLEDocument
    ----- ------------

    NOTE: we only have one object per document. if your app allowed multiple
          objects per document then you would have a list of objects...
*/
_CLASSDEF (TOLEDocument)

class TOLEDocument  : public OLESERVERDOC {
  public:
    LHSERVERDOC    lhDoc;     // registration handle returned by server library
    DOCTYPE        type;
    char          *szName;
    TOLEObject    *pObject;
    BOOL           fDirty;
    BOOL           fRelease;  // TRUE if Release method has been called

    //
    // if "szPath" is NULL then we create an untitled document and default object
    //
    // the type is "doctypeNew" if "lhDoc" is NULL and "doctypeEmbedded" if
    // "lhDoc" is non NULL
    //
    // if "szPath" is non NULL we create a document of type "doctypeFromFile"
    // and initialize it from file "szPath"
    //
    // if "lhDoc" is NULL then we call OleRegisterServerDoc() otherwise we
    // just use "lhDoc" as our registration handle
    //
    // you specify if the document is initially marked as "dirty"
    //
	  TOLEDocument (TOLEServer &server,
                  LHSERVERDOC lhDoc = 0,
                  LPSTR       szPath = 0,
                  BOOL        dirty = FALSE);

    void          SaveDoc ();  // save doc to "name"
    void          SaveAs ();   // prompt user for file to save to...

    //
    // if "szPath" is NULL then resets document to "doctypeNew"; otherwise
    // streams in the contents of "file" and sets the type to "doctypeFile"
    //
    // marks the document as not modified
    //
    void          Reset (LPSTR szPath);

    //
    // changes "szName" and also changes the window's title bar (caption) if
    // "changeCaption" is TRUE...
    //
    void          SetDocumentName (LPSTR, BOOL changeCaption = TRUE);

    //
    // prompts user for filename to open and returns the string in "szPath"
    //
    static BOOL   PromptForOpenFileName (char *szPath);

    //
    // vtable initialization
    //
    static BOOL   InitVTBL (HINSTANCE);

    //
    // vtable entries
    //
    static OLESTATUS FAR PASCAL _export Save (LPOLESERVERDOC);
    static OLESTATUS FAR PASCAL _export Close (LPOLESERVERDOC);
    static OLESTATUS FAR PASCAL _export SetHostNames (LPOLESERVERDOC, LPSTR, LPSTR);
    static OLESTATUS FAR PASCAL _export SetDocDimensions (LPOLESERVERDOC, LPRECT);
    static OLESTATUS FAR PASCAL _export GetObject (LPOLESERVERDOC, LPSTR, LPOLEOBJECT FAR *, LPOLECLIENT);
    static OLESTATUS FAR PASCAL _export Release (LPOLESERVERDOC);
    static OLESTATUS FAR PASCAL _export SetColorScheme (LPOLESERVERDOC, LPLOGPALETTE);
    static OLESTATUS FAR PASCAL _export Execute (LPOLESERVERDOC, HANDLE);

  private:
    static OLESERVERDOCVTBL  _vtbl;

    BOOL          LoadFromFile (LPSTR);
};


/*
    class TOLEServer
    ----- ----------

    NOTE: we only have one document per server. if your app was MDI then you
          would have a list of documents...
*/
class TOLEServer : public OLESERVER {
  public:
    LHSERVER       lhServer;  // registration handle returned by server library
    TOLEDocument  *pDocument;
    BOOL           fRelease;  // TRUE if Release method has been called

    //
    // registers with the server library
    //
    // if "embedded" is FALSE, creates an untitled document; otherwise it
    // waits for the server library to request document creation
    //
	  TOLEServer (TOLEApp &, BOOL embedded);

    //
    // registers with the server library
    //
    // creates a document of type "objtypeFromFile" which is initialized from
    // file "szPath"
    //
    TOLEServer (TOLEApp &, LPSTR szPath);

    //
    // vtable initialization
    //
    static BOOL   InitVTBL (HINSTANCE);

    //
    // vtable entries
    //
    static OLESTATUS FAR PASCAL _export Open (LPOLESERVER, LHSERVERDOC, LPSTR, LPOLESERVERDOC FAR *);
    static OLESTATUS FAR PASCAL _export Create (LPOLESERVER, LHSERVERDOC, LPSTR,LPSTR, LPOLESERVERDOC FAR *);
    static OLESTATUS FAR PASCAL _export CreateFromTemplate (LPOLESERVER, LHSERVERDOC, LPSTR, LPSTR, LPSTR, LPOLESERVERDOC FAR *);
    static OLESTATUS FAR PASCAL _export Edit (LPOLESERVER, LHSERVERDOC, LPSTR, LPSTR, LPOLESERVERDOC FAR *);
    static OLESTATUS FAR PASCAL _export Exit (LPOLESERVER);
    static OLESTATUS FAR PASCAL _export Release (LPOLESERVER);
    static OLESTATUS FAR PASCAL _export Execute (LPOLESERVER, HANDLE);

  private:
    static OLESERVERVTBL  _vtbl;

    BOOL          Init (TOLEApp &);
    BOOL          RegisterWithDatabase ();
    BOOL          WantsToRegister ();
};


/*
    class TWindowServer
    ----- -------------
*/
_CLASSDEF (TWindowServer)

class TWindowServer: public TWindow {
  public:
    TWindowServer (PTWindowsObject, LPSTR);

    //
    // handler for "Help" menu items
    //
    void           About (RTMessage) = [CM_FIRST + IDM_ABOUT];

    //
    // handlers for "File" menu items
    //
    void           New (RTMessage) = [CM_FIRST + IDM_NEW];
    void           Open (RTMessage) = [CM_FIRST + IDM_OPEN];
    void           Save (RTMessage) = [CM_FIRST + IDM_SAVE];
    void           Update (RTMessage) = [CM_FIRST + IDM_UPDATE];
    void           SaveAs (RTMessage) = [CM_FIRST + IDM_SAVEAS];

    //
    // handler for "Edit" menu items
    //
    void           Copy (RTMessage) = [CM_FIRST + IDM_COPY];

    //
    // routines to handle switching between embedding mode and stand-alone
    // mode
    //
    void           BeginEmbedding ();
    void           EndEmbedding ();

    //
    // alter File/Save and File/Exit to reflect the name of the client's
    // application document (only called for embedded objects)
    //
    void           UpdateFileMenu (LPSTR szDocName);

    void           WMCommand (RTMessage);
    BOOL           CanClose();
    //
    // gets called by WMCommand() when the user selects a menu item from the
    // "Shape" menu
    //
    void           ShapeChange (NATIVETYPE);

    void           Paint (HDC, PAINTSTRUCT _FAR &);

  protected:
    //
    // routine to prompt the user to save changes in a document before creating
    // a new one, opening a new file, or exiting the application
    //
    // - choosing "Yes" causes the document to be saved and returns "fiExecute"
    // - choosing "No" discards the document and also returns "fiExecute"
    // - choosing "Cancel" discards the document and returns "fiCancel"
    //
    FILEIOSTATUS   SaveChangesPrompt ();
};


/*
    class TOLEApp
    ----- -------
*/
class TOLEApp : public TApplication {
  public:
    TOLEServer    *pServer;
    OLECLIPFORMAT  cfNative, cfOwnerLink, cfObjectLink;

    TOLEApp (LPSTR      name,
             HINSTANCE  hInstance,
             HINSTANCE  hPrevInstance,
             LPSTR      lpCmdLine,
             int        nCmdShow) : TApplication (name,
                                                  hInstance,
                                                  hPrevInstance,
                                                  lpCmdLine,
                                                  nCmdShow) {}

    void     InitInstance ();

    void     Wait (BOOL &waitFlag);

  protected:
    BOOL     RegisterClipboardFormats ();
    void     CreateServer ();
    void     Error (int ErrorCode);
};

