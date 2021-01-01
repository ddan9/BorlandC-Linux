/*****************************************************************************\
*                                                                             *
* penwoem.h -   Pen Windows APIs into recognizer layer.                       *
*                                                                             *
*               Version 1.0                                                   *
*                                                                             *
\*****************************************************************************/

#ifndef __PENWOEM_H     /* prevent multiple includes */
#define __PENWOEM_H

#ifndef __WINDOWS_H
#include <windows.h>    /* <windows.h> must be included */
#endif  /* __WINDOWS_H */

#ifndef __PENWIN_H
#include <penwin.h>     /* <penwin.h> must be included */
#endif  /* __PENWIN_H */

#ifndef RC_INVOKED
#pragma option -a-      /* Assume byte packing throughout */
#endif  /* RC_INVOKED */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

typedef int (CALLBACK *LPFUNCRESULTS) (LPRCRESULT, REC);

/* Initialization Functions */

#define WCR_RECOGNAME          0
#define WCR_QUERY              1
#define WCR_CONFIGDIALOG       2
#define WCR_DEFAULT            3
#define WCR_RCCHANGE           4
#define WCR_VERSION            5
#define WCR_TRAIN              6
#define WCR_TRAINSAVE          7
#define WCR_TRAINMAX           8
#define WCR_TRAINDIRTY         9
#define WCR_TRAINCUSTOM        10
#define WCR_QUERYLANGUAGE      11
#define WCR_USERCHANGE         12
#define WCR_PRIVATE            1024

/* sub-function of WCR_USERCHANGE */
#define CRUC_REMOVE            1

/* Return values for WCR_TRAIN Function */
#define TRAIN_NONE             0x0000
#define TRAIN_DEFAULT          0x0001
#define TRAIN_CUSTOM           0x0002
#define TRAIN_BOTH             (TRAIN_DEFAULT | TRAIN_CUSTOM)

/* Control values for TRAINSAVE */
#define TRAIN_SAVE             0  /* Save changes that have been made */
#define TRAIN_REVERT           1  /* Discard changes that have been made */

UINT WINAPI ConfigRecognizer(UINT, WPARAM, LPARAM);
BOOL WINAPI InitRecognizer(LPRC);
VOID WINAPI CloseRecognizer(VOID);

/* Recognition Functions */
REC  WINAPI RecognizeInternal(LPRC, LPFUNCRESULTS);
REC  WINAPI RecognizeDataInternal(LPRC, HPENDATA, LPFUNCRESULTS);

/* Training Functions */
BOOL WINAPI TrainInkInternal(LPRC, HPENDATA, LPSYV);
BOOL WINAPI TrainContextInternal(LPRCRESULT, LPSYE, int, LPSYC, int);

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

#ifndef RC_INVOKED
#pragma option -a.      /* Revert to default packing */
#endif  /* RC_INVOKED */

#endif  /* __PENWOEM_H */
