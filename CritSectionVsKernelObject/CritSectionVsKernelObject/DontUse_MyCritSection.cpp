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

#include "DontUse_MyCritSection.h"



//This class implements its "own" critical section using a kernel event object
//
//IMPORTANT: DO NOT use this class due to its bad performance!
//           (Read blog post above for details.)
//

// This is a re-creation of the original class.
// In it, the original author resorted to using native function calls, presumably
// to improve performance of the following code...
//



/// <summary>
/// Initialize a critical section object
/// </summary>
/// <param name="pCS">Critical section object to fill out</param>
/// <param name="bInitEvent">TRUE to pre-allocate a synchronization event</param>
/// <returns>TRUE if success</returns>
BOOL DontUse_MyCritSection::InitializeCriticalSection(__out MY_CRIT_SEC* pCS, __in BOOL bInitEvent) 
{
    assert(pCS);

    //Initialize our critical section object
    *pCS = MY_CRIT_SEC();

    if (bInitEvent)
    {
        if(_getSynchEvent(pCS) == NULL)
        {
            //Failed to create an event
            assert(false);

            return FALSE;
        }
    }

    return TRUE;
}



/// <summary>
/// Delete critical section object
/// </summary>
/// <param name="pCS">Critical section object to delete</param>
/// <returns>TRUE if no errors</returns>
BOOL DontUse_MyCritSection::DeleteCriticalSection(__in MY_CRIT_SEC* pCS)
{
    assert(pCS);

    BOOL bRes = TRUE;

    if (pCS->hEvent != 0)
    {
        //Close our synchronization event
        if(NtClose(pCS->hEvent) != STATUS_SUCCESS)
        {
            //Error
            assert(false);
            bRes = FALSE;
        }
    }

    return bRes;
}



/// <summary>
/// Create a synchronization event, or use a previously created one
/// </summary>
/// <param name="pCS">Critical section object</param>
/// <returns>Event handle (no need to close it!), or NULL if error</returns>
HANDLE DontUse_MyCritSection::_getSynchEvent(MY_CRIT_SEC* pCS)
{
    assert(pCS);

    //Only if we don't have it already
    if (pCS->hEvent != NULL)
    {
        //If so, return an existing event
        return pCS->hEvent;
    }

    //Create a synchronization event
    HANDLE hEvent = NULL;
    if(ZwCreateEvent(&hEvent, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, FALSE) != STATUS_SUCCESS)
    {
        //Error
        assert(false);
        return NULL;
    }

    //Check if we already have an event
    //INFO: This may happen if more than one thread attempts to call this function...
    HANDLE hOrig = InterlockedCompareExchangePointer(&pCS->hEvent, hEvent, NULL);
    if(hOrig != NULL)
    {
        //Previous event already existed, let's close one
        NtClose(hEvent);
        return hOrig;
    }

    return hEvent;
}





/// <summary>
/// Enter this critical section (and potentially wait for it to be available)
/// </summary>
/// <param name="pCS">Critical section object to enter</param>
/// <param name="dwTimeout">Timeout in ms, or 0 not to wait, or INFINITE to wait for as long as it takes</param>
/// <returns>NTSTATUS with result. Use SUCCEEDED() macro to check.</returns>
NTSTATUS DontUse_MyCritSection::EnterCriticalSection(__in MY_CRIT_SEC* pCS, __in DWORD dwTimeout) 
{
    NTSTATUS status = STATUS_SUCCESS;

    //Since we're implementing it ourselves, we need to account for a special case when we called ExitProcess
    //and some other threads that hold our lock might have been forcefully closed.
    //
    // Read the following blog post for details:
    //
    //  https://dennisbabkin.com/blog/?i=AAA11C10
    //
    if(!RtlDllShutdownInProgress())
    {
        //Get current thread ID
        DWORD dwThreadId = GetCurrentThreadId();

        if(InterlockedIncrement(&pCS->nLockCount) == 0)
        {
            //No thread owns this critical section - we're the first owner
            pCS->dwThreadId = dwThreadId;
            pCS->nRecursiveCount = 1;
        }
        else
        {
            //Some thread owns it
            if(pCS->dwThreadId == dwThreadId)
            {
                //We already own it - increment recursive count
                pCS->nRecursiveCount++;
            }
            else
            {
                //Another thread owns it

                //Determine how long to wait
                PLARGE_INTEGER pli = NULL;
                LARGE_INTEGER li;
                if(dwTimeout != INFINITE)
                {
                    //Translate from 100ns intervals to ms
                    li.QuadPart = dwTimeout * 10LL;
                    pli = &li;
                }

                //Use event to wait for
                HANDLE hEvent = _getSynchEvent(pCS);

                //Wait for the owning thread to release it
                status = NtWaitForSingleObject(hEvent, FALSE, pli);

                if (status == STATUS_SUCCESS)
                {
                    //We got the ownership of the critical section
                    pCS->dwThreadId = dwThreadId;
                    pCS->nRecursiveCount = 1;
                }
            }
        }
    }

    return status;
}




/// <summary>
/// Release this critical section that was previously acquired by a call to DontUse_MyCritSection::EnterCriticalSection()
/// </summary>
/// <param name="pCS">Critical section object to leave</param>
void DontUse_MyCritSection::LeaveCriticalSection(__in MY_CRIT_SEC* pCS) 
{
    assert(pCS);

    //Since we're implementing it ourselves, we need to account for a special case when we called ExitProcess
    //and some other threads that hold our lock might have been forcefully closed.
    //
    // Read the following blog post for details:
    //
    //  https://dennisbabkin.com/blog/?i=AAA11C10
    //
    if(!RtlDllShutdownInProgress())
    {
        pCS->nRecursiveCount--;
        if(pCS->nRecursiveCount > 0)
        {
            //We still own it
            InterlockedDecrement(&pCS->nLockCount);
        }
        else
        {
            //We don't own the critical section anymore
            pCS->dwThreadId = 0;
            if(InterlockedDecrement(&pCS->nLockCount) >= 0)
            {
                //Other threads are waiting, wake one on them
                NTSTATUS status = ZwSetEvent(_getSynchEvent(pCS), NULL);
                if(!NT_SUCCESS(status))
                {
                    //Critical error
                    assert(false);
                    __fastfail(FAST_FAIL_INVALID_LOCK_STATE);
                }
            }
        }
    }
}







