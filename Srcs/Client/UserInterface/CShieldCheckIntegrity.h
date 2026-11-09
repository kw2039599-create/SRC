#pragma once
#include <windows.h>
#include <cassert>
#include "CShieldStringEncryption.h"

#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS) 0xC0000004)

typedef LONG NTSTATUS;

BOOL ValidateCShield();
#ifdef ENABLE_CSHIELD_DEBUG
void ErrorMessage(std::string error);
#endif

enum KWAIT_REASON
{
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel,
    MaximumWaitReason
};

enum THREAD_STATE
{
    Running = 2,
    Waiting = 5,
};

#pragma pack(push,8)

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessInformation = 5,
    SystemProcessorPerformanceInformation = 8,
    SystemInterruptInformation = 23,
    SystemExceptionInformation = 33,
    SystemRegistryQuotaInformation = 37,
    SystemLookasideInformation = 45,
    SystemCodeIntegrityInformation = 103,
    SystemPolicyInformation = 134,
} SYSTEM_INFORMATION_CLASS;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING* PUNICODE_STRING;
typedef const UNICODE_STRING* PCUNICODE_STRING;

struct CLIENT_ID
{
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
};

struct SYSTEM_THREAD
{
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG         WaitTime;
    PVOID         StartAddress;
    CLIENT_ID     ClientID;
    LONG          Priority;
    LONG          BasePriority;
    ULONG         ContextSwitches;
    THREAD_STATE  ThreadState;
    KWAIT_REASON  WaitReason;
};

struct VM_COUNTERS
{
    ULONG_PTR PeakVirtualSize;
    ULONG_PTR VirtualSize;
    ULONG     PageFaultCount;
    ULONG_PTR PeakWorkingSetSize;
    ULONG_PTR WorkingSetSize;
    ULONG_PTR QuotaPeakPagedPoolUsage;
    ULONG_PTR QuotaPagedPoolUsage;
    ULONG_PTR QuotaPeakNonPagedPoolUsage;
    ULONG_PTR QuotaNonPagedPoolUsage;
    ULONG_PTR PagefileUsage;
    ULONG_PTR PeakPagefileUsage;
};

struct SYSTEM_PROCESS
{
    ULONG          NextEntryOffset;
    ULONG          ThreadCount;
    LARGE_INTEGER  WorkingSetPrivateSize;
    ULONG          HardFaultCount;
    ULONG          NumberOfThreadsHighWatermark;
    ULONGLONG      CycleTime;
    LARGE_INTEGER  CreateTime;
    LARGE_INTEGER  UserTime;
    LARGE_INTEGER  KernelTime;
    UNICODE_STRING ImageName;
    LONG           BasePriority;
    PVOID          UniqueProcessId;
    PVOID          InheritedFromUniqueProcessId;
    ULONG          HandleCount;
    ULONG          SessionId;
    ULONG_PTR      UniqueProcessKey;
    VM_COUNTERS    VmCounters;
    ULONG_PTR      PrivatePageCount;
    IO_COUNTERS    IoCounters;
};

#pragma pack(pop)

typedef NTSTATUS(WINAPI* t_NtQueryInfo)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);

class cProcInfo
{
public:
    cProcInfo()
    {
#ifdef _WIN64
        assert(sizeof(SYSTEM_THREAD) == 0x50 && sizeof(SYSTEM_PROCESS) == 0x100);
#else
        assert(sizeof(SYSTEM_THREAD) == 0x40 && sizeof(SYSTEM_PROCESS) == 0xB8);
#endif

        mu32_DataSize = 1000;
        mp_Data = NULL;
        mf_NtQueryInfo = NULL;
    }
    virtual ~cProcInfo()
    {
        if (mp_Data) LocalFree(mp_Data);
    }

    DWORD Capture()
    {
        if (!mf_NtQueryInfo)
        {
            mf_NtQueryInfo = (t_NtQueryInfo)GetProcAddress(GetModuleHandleA("NtDll.dll"), "NtQuerySystemInformation");
            if (!mf_NtQueryInfo)
                return GetLastError();
        }

        while (true)
        {
            if (!mp_Data)
            {
                mp_Data = (BYTE*)LocalAlloc(LMEM_FIXED, mu32_DataSize);
                if (!mp_Data)
                    return GetLastError();
            }

            ULONG u32_Needed = 0;
            NTSTATUS s32_Status = mf_NtQueryInfo(SystemProcessInformation, mp_Data, mu32_DataSize, &u32_Needed);

            if (s32_Status == STATUS_INFO_LENGTH_MISMATCH)
            {
                mu32_DataSize = u32_Needed + 4000;
                LocalFree(mp_Data);
                mp_Data = NULL;
                continue;
            }
            return s32_Status;
        }
    }

    SYSTEM_PROCESS* FindProcessByPid(DWORD u32_PID)
    {
        if (!mp_Data)
        {
            assert(mp_Data);
            return NULL;
        }

        SYSTEM_PROCESS* pk_Proc = (SYSTEM_PROCESS*)mp_Data;
        while (TRUE)
        {
            if ((DWORD)(DWORD_PTR)pk_Proc->UniqueProcessId == u32_PID)
                return pk_Proc;

            if (!pk_Proc->NextEntryOffset)
                return NULL;

            pk_Proc = (SYSTEM_PROCESS*)((BYTE*)pk_Proc + pk_Proc->NextEntryOffset);
        }
    }

    SYSTEM_THREAD* FindThreadByTid(SYSTEM_PROCESS* pk_Proc, DWORD u32_TID)
    {
        if (!pk_Proc)
        {
            assert(pk_Proc);
            return NULL;
        }

        SYSTEM_THREAD* pk_Thread = (SYSTEM_THREAD*)((BYTE*)pk_Proc + sizeof(SYSTEM_PROCESS));

        for (DWORD i = 0; i < pk_Proc->ThreadCount; i++)
        {
            if (pk_Thread->ClientID.UniqueThread == (HANDLE)(DWORD_PTR)u32_TID)
                return pk_Thread;

            pk_Thread++;
        }
        return NULL;
    }

    DWORD IsThreadSuspended(SYSTEM_THREAD* pk_Thread, BOOL* pb_Suspended)
    {
        if (!pk_Thread)
            return ERROR_INVALID_PARAMETER;

        *pb_Suspended = (pk_Thread->ThreadState == Waiting &&
            pk_Thread->WaitReason == Suspended);
        return 0;
    }

private:
    BYTE* mp_Data;
    DWORD       mu32_DataSize;
    t_NtQueryInfo mf_NtQueryInfo;
};

__forceinline void CShieldClose()
{
#ifdef ENABLE_CSHIELD_DEBUG
    ErrorMessage("client close");
#endif
    __fastfail(0);
    _exit(0);
    abort();
    std::terminate();
#ifndef _WIN64
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)NULL);
    __asm {
        mov eax, 0x001
        jmp eax
    }
#endif
}
