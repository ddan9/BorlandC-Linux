// ObjectWindows - (C) Copyright 1992 by Borland International
//
// tscrnsav.h

#ifndef  __TSCRNSAV_H
#define  __TSCRNSAV_H

#ifndef  __OWL_H
#include <owl.h>
#endif

#define  WM_SAVESCREEN   ( WM_USER + 500 )


_CLASSDEF( TScrnSavWindow )
class TScrnSavWindow : public TWindow
{
        POINT   prevPt;
    public:
        TScrnSavWindow( PTWindowsObject AParent, LPSTR ATitle,
                        PTModule AModule = NULL );
       ~TScrnSavWindow();
        virtual LPSTR GetClassName(){ return "_OWLScreenSaverClass"; }
        virtual void GetWindowClass( WNDCLASS & AWndClass );
        virtual void SetupWindow( void );
        virtual void DefWndProc( RTMessage );
        virtual void WMSysCommand( RTMessage ) = [ WM_FIRST + WM_SYSCOMMAND ];
        virtual void AnimateScreen();
};





_CLASSDEF( TScrnSavApp )
class TScrnSavApp : public TApplication
{
    private:
        virtual void InitMainWindow();
    protected:
        BOOL     fConfigureFlag;
        PTDialog pConfigureDialog;
        PTScrnSavWindow pScrnSavWnd;
    public:
        TScrnSavApp( LPSTR AName, HINSTANCE AnInstance,
                     HINSTANCE APrevInstance,
                     LPSTR ACmdLine, int ACmdShow ) :
        TApplication( AName, AnInstance, APrevInstance, ACmdLine, ACmdShow )
        {
            pScrnSavWnd = NULL;
            pConfigureDialog = NULL;
        }
        virtual void IdleAction();
        virtual void InitScrnSavWindow();
        virtual void InitConfigDialog(){};
};
#endif // __TSCRNSAV_H
