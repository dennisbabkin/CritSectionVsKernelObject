// This is a Proof-of-Concept (POC) project that demonstrates
// the efficiency of use of the Windows critical section
// versus a kernel synchronization object.
//
// Copyright (c) 2023, by dennisbabkin.com
//
//
// This project is used in the following blog post:
//
//  "Critical Section vs Kernel Objects"
//  "Spinning in user-mode versus entering kernel - the cost of a SYSCALL in Windows."
//
//   https://dennisbabkin.com/blog/?i=AAA11D00
//

#pragma once

#include <Windows.h>            //Win32 stuff
#include <assert.h>
#include "NativeStuff.h"        //NT internals
#include <intrin.h>




//Data structure for this implementation of a critical section
struct MY_CRIT_SEC
{
    LONG   nLockCount;
    DWORD  dwThreadId;
    LONG   nRecursiveCount;
    HANDLE hEvent;

    MY_CRIT_SEC()
        : nLockCount(-1)        //No threads have entered it
        , dwThreadId(0)         //Not owned by any thread
        , nRecursiveCount(0)
        , hEvent(NULL)          //Will be allocated on demand
    {
    }
};




//Some debugging macros
#ifdef _DEBUG
#define verify(f) assert(f)
#else
#define verify(f) (f)
#endif



//This class implements its "own" critical section using a kernel event object
//
//IMPORTANT: DO NOT use this class due to its bad performance!
//           (Read blog post above for details.)
//
struct DontUse_MyCritSection
{
    static BOOL InitializeCriticalSection(__out MY_CRIT_SEC* pCS, __in BOOL bInitEvent = FALSE);
    static BOOL DeleteCriticalSection(__in MY_CRIT_SEC* pCS);
    static NTSTATUS EnterCriticalSection(__in MY_CRIT_SEC* pCS, __in DWORD dwTimeout);
    static void LeaveCriticalSection(__in MY_CRIT_SEC* pCS);

private:
    static HANDLE _getSynchEvent(MY_CRIT_SEC* pCS);

};

