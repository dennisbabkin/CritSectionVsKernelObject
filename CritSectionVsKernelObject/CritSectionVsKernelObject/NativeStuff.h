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


#include <winternl.h>           //NT internals
#include <winnt.h>              //NT stuff


#pragma comment(lib, "ntdll.lib")




typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
} EVENT_TYPE;



//Native APIs that will be statically linked to ntdll.dll
//
extern "C" 
{
    BOOLEAN 
        NTAPI
        RtlDllShutdownInProgress();

    NTSTATUS
        NTAPI
        ZwCreateEvent(
            _Out_ PHANDLE EventHandle,
            _In_ ACCESS_MASK DesiredAccess,
            _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
            _In_ EVENT_TYPE EventType,
            _In_ BOOLEAN InitialState
        );

    NTSTATUS
        NTAPI
        NtClose(
            _In_ _Post_ptr_invalid_ HANDLE Handle
        );

    NTSTATUS
        NTAPI
        ZwWaitForSingleObject(
            _In_ HANDLE Handle,
            _In_ BOOLEAN Alertable,
            _In_opt_ PLARGE_INTEGER Timeout
        );

    NTSTATUS
        NTAPI
        ZwSetEvent(
            _In_ HANDLE EventHandle,
            _Out_opt_ PLONG PreviousState
        );

};



#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif



