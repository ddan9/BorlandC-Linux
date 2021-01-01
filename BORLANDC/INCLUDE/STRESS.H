/*****************************************************************************\
*                                                                             *
* stress.h -    Stress functions definitions                                  *
*                                                                             *
*               Version 1.0                                                   *
*                                                                             *
*******************************************************************************/

#ifndef __STRESS_H      /* prevent multiple includes */
#define __STRESS_H

#ifndef __WINDOWS_H
#include <windows.h>    /* <windows.h> must be included */
#endif  /* __WINDOWS_H */

#ifndef RC_INVOKED
#pragma option -a-      /* Assume byte packing throughout */
#endif  /* RC_INVOKED */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/****** Simple types & common helper macros *********************************/

/* If included with the 3.0 windows.h, define compatible aliases */
#if !defined(WINVER) || (WINVER < 0x030a)
#define UINT        WORD
#define WINAPI      FAR PASCAL
#endif  /* WIN3.0 */

/* stuff for AllocDiskSpace() */
#define  EDS_WIN     1
#define  EDS_CUR     2
#define  EDS_TEMP    3


/* function prototypes */
BOOL    WINAPI AllocMem(DWORD);
void    WINAPI FreeAllMem(void);
int     WINAPI AllocFileHandles(int);
void    WINAPI UnAllocFileHandles(void);
int     WINAPI GetFreeFileHandles(void);
int     WINAPI AllocDiskSpace(long,UINT);
void    WINAPI UnAllocDiskSpace(UINT);
BOOL    WINAPI AllocUserMem(UINT);
void    WINAPI FreeAllUserMem(void);
BOOL    WINAPI AllocGDIMem(UINT);
void    WINAPI FreeAllGDIMem(void);

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

#ifndef RC_INVOKED
#pragma option -a.      /* Revert to default packing */
#endif  /* RC_INVOKED */

#endif  /* __STRESS_H */
