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

#include <iostream>

#include "DontUse_MyCritSection.h"          //Class with a home-made custom implementation of a critical section



CRITICAL_SECTION g_CS;                      //Default implementation of the critical section in the OS
MY_CRIT_SEC g_myCS;                         //Custom implementation of the critical section using a kernel object for synchronization

#define NUM_WORK_THREADS 5                  //Number of worker threads for this test
#define ITERATIONS_PER_WORK_THREAD  500000  //Iterations in each thread



void runTimingTest(bool bUseCustomCriticalSection);
DWORD WINAPI ThreadProc_MainTest(LPVOID lpParameter);
DWORD WINAPI ThreadProc_Checker(LPVOID lpParameter);




int main()
{
#ifdef _DEBUG
    std::cout << 
        "WARNING: It's probably better to run this test in an optimized Release configuration build..." << 
        std::endl << 
        std::endl;
#endif

    //Initialize our two critical sections
    InitializeCriticalSection(&g_CS);
    DontUse_MyCritSection::InitializeCriticalSection(&g_myCS);


    //First run the test with the OS-provided critical section
    runTimingTest(false);

    std::cout << "=========================================" << std::endl << std::endl;

    //Next run the test with the custom critical section
    runTimingTest(true);

    std::cout << std::endl;


    //Delete critical section objects
    DeleteCriticalSection(&g_CS);
    DontUse_MyCritSection::DeleteCriticalSection(&g_myCS);

    return 0;
}




//Made up some global buffer to provide synchronized access to
int gBuffInts[256];
int gIdx = 0;
volatile bool gbTestIsDone;



void runTimingTest(bool bUseCustomCriticalSection)
{
    std::cout << "Starting test with a " << 
        (bUseCustomCriticalSection ? "custom" : "built-in") << 
        " critical section:" << std::endl;

    std::cout << "Threads: " << NUM_WORK_THREADS << std::endl;
    std::cout << "Iterations per thread: " << ITERATIONS_PER_WORK_THREAD << std::endl;

    //Begin timing
    DWORD dwmsIniTicks = GetTickCount();

    //Reset our test buffer
    gIdx = 0;
    memset(gBuffInts, 0, sizeof(gBuffInts));
    gbTestIsDone = false;


    //Multi-threaded test
    HANDLE hThreadWorkers[NUM_WORK_THREADS] = {};
    for(intptr_t i = 0; i < _countof(hThreadWorkers); i++)
    {
        hThreadWorkers[i] = CreateThread(NULL, 0, ThreadProc_MainTest, (LPVOID)bUseCustomCriticalSection, 0, NULL);
        assert(hThreadWorkers[i]);
    }

    //And start another thread that will check our global buffer for consistency
    HANDLE hCheckerThread = CreateThread(NULL, 0, ThreadProc_Checker, (LPVOID)bUseCustomCriticalSection, 0, NULL);
    assert(hCheckerThread);


    //Wait for all test threads to finish
    for(intptr_t i = 0; i < _countof(hThreadWorkers); i++)
    {
        assert(hThreadWorkers[i]);
        verify(WaitForSingleObject(hThreadWorkers[i], INFINITE) == WAIT_OBJECT_0);

        verify(CloseHandle(hThreadWorkers[i]));
    }


    //Mark the global variable that the test is done!
    gbTestIsDone = true;


    //Wait for the checker thread
    assert(hCheckerThread);
    verify(WaitForSingleObject(hCheckerThread, INFINITE) == WAIT_OBJECT_0);
    verify(CloseHandle(hCheckerThread));


    //See how long it took
    DWORD dwmsElapsed = GetTickCount() - dwmsIniTicks;

    std::cout << "Runtime: " << dwmsElapsed << " ms" << std::endl;

}




/// <summary>
/// A thread to run the main performance test
/// </summary>
/// <param name="lpParameter">Not-0 to use a custom critical section, or 0 to use a default critical section from the OS</param>
/// <returns></returns>
DWORD WINAPI ThreadProc_MainTest(LPVOID lpParameter)
{
    bool bUseCustomCriticalSection = !!lpParameter;

    for(intptr_t t = 0; t < ITERATIONS_PER_WORK_THREAD; t++)
    {
        //Enter critical section
        ///////////////////////////////////////////////////////
        if(bUseCustomCriticalSection)
        {
            //Use our custom critical section
            verify(SUCCEEDED(DontUse_MyCritSection::EnterCriticalSection(&g_myCS, INFINITE)));
        }
        else
        {
            //Use OS-provided critical section
            EnterCriticalSection(&g_CS);
        }


        //Set our buffer with values in a specific order to check if the lock holds
        int i = gIdx;
        if(i > 0)
        {
            gBuffInts[i] = gBuffInts[i - 1] + 17 + i;
        }
        else
        {
            //Set the initial value to some basic pseudo-random number
            gBuffInts[0] = GetTickCount() & 0xFFFFFF;
        }

        gIdx++;
        if(gIdx > _countof(gBuffInts))
        {
            gIdx = 0;
        }


        //Leave critical section
        ///////////////////////////////////////////////////////
        if(bUseCustomCriticalSection)
        {
            //Use our custom critical section
            DontUse_MyCritSection::LeaveCriticalSection(&g_myCS);
        }
        else
        {
            //Use OS-provided critical section
            LeaveCriticalSection(&g_CS);
        }
    }

    return 0;
}





/// <summary>
/// Thread that checks our global buffer for consistency (to make sure that our synchronization locks work)
/// </summary>
/// <param name="lpParameter">Not-0 to use a custom critical section, or 0 to use a default critical section from the OS</param>
/// <returns></returns>
DWORD WINAPI ThreadProc_Checker(LPVOID lpParameter)
{
    //This thread checks the buffer for consistency
    //INFO: We do this to check that our locks hold...

    bool bUseCustomCriticalSection = !!lpParameter;

    //We do it continuously while the tests are running
    //INFO: Do it with a 1 ms delay though...
    //
    for(; !gbTestIsDone; Sleep(1))
    {
        //Enter critical section
        ///////////////////////////////////////////////////////
        if(bUseCustomCriticalSection)
        {
            //Use our custom critical section
            verify(SUCCEEDED(DontUse_MyCritSection::EnterCriticalSection(&g_myCS, INFINITE)));
        }
        else
        {
            //Use OS-provided critical section
            EnterCriticalSection(&g_CS);
        }


        //Check buffer for correctness
        for(int i = 0; i < gIdx; i++)
        {
            int v1 = gBuffInts[i];
            if(i + 1 < gIdx)
            {
                int v2 = gBuffInts[i + 1];

                if(v2 != v1 + 17 + i + 1)
                {
                    //Buffer is corrupted!
                    //INFO: If we get here - this means that our implementation of the lock is incorrect!
                    std::cout << "CRITICAL ERROR: Buffer inconsistency - the lock does not work!!!" << std::endl;
                    assert(false);
                    exit(-1);
                }
            }
        }


        //Leave critical section
        ///////////////////////////////////////////////////////
        if(bUseCustomCriticalSection)
        {
            //Use our custom critical section
            DontUse_MyCritSection::LeaveCriticalSection(&g_myCS);
        }
        else
        {
            //Use OS-provided critical section
            LeaveCriticalSection(&g_CS);
        }
    }

    return 0;
}
