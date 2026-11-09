#include "stdafx.h"
#if defined(ENABLE_CSHIELD) && !defined(__ANDROID__)
#include <iostream>
#include <fstream>
#include <psapi.h>
#include "CShield.h"
#include "CShieldCheckIntegrity.h"
#endif

#ifdef ENABLE_CSHIELD
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define ThreadQuerySetWin32StartAddress 9
#define NtCurrentProcess ((HANDLE)(LONG_PTR)-1)
typedef NTSTATUS(WINAPI* NTQUERYINFOMATIONTHREAD)(HANDLE, LONG, PVOID, ULONG, PULONG);

DWORD_PTR WINAPI GetThreadStartAddress(__in HANDLE hThread)
{
	NTSTATUS ntStatus;
	DWORD_PTR dwThreadStartAddr = 0;
	HANDLE hPeusdoCurrentProcess, hNewThreadHandle;
	NTQUERYINFOMATIONTHREAD NtQueryInformationThread;

	if ((NtQueryInformationThread = (NTQUERYINFOMATIONTHREAD)GetProcAddress(GetModuleHandle("ntdll.dll"), "NtQueryInformationThread"))) {
		hPeusdoCurrentProcess = GetCurrentProcess();
		if (DuplicateHandle(hPeusdoCurrentProcess, hThread, hPeusdoCurrentProcess, &hNewThreadHandle, THREAD_QUERY_INFORMATION, FALSE, 0)) {
			ntStatus = NtQueryInformationThread(hNewThreadHandle, ThreadQuerySetWin32StartAddress, &dwThreadStartAddr, sizeof(DWORD_PTR), NULL);
			CloseHandle(hNewThreadHandle);
			if (ntStatus != STATUS_SUCCESS) return 0;
		}
	}

	return dwThreadStartAddr;
}

BOOL ValidateCShield()
{
	BOOL b_Suspend = false;;
	cProcInfo i_Proc;
	DWORD u32_Error = i_Proc.Capture();

	if (u32_Error)
	{
#ifdef ENABLE_CSHIELD_DEBUG
		ErrorMessage("Error 1");
#endif
		return true;
	}

	SYSTEM_PROCESS* pk_Proc = i_Proc.FindProcessByPid(GetCurrentProcessId());
	if (!pk_Proc)
	{
#ifdef ENABLE_CSHIELD_DEBUG
		ErrorMessage("Couldnt find PID");
#endif
		cshieldClientCode = 42;
		CShieldClose();
	}

	SYSTEM_THREAD* pk_Thread = i_Proc.FindThreadByTid(pk_Proc, myCShieldData.dwThreadId);
	if (!pk_Thread)
	{
#ifdef ENABLE_CSHIELD_DEBUG
		ErrorMessage("Couldnt find TID");
#endif
		cshieldClientCode = 42;
		CShieldClose();
	}

	i_Proc.IsThreadSuspended(pk_Thread, &b_Suspend);
	if (b_Suspend)
	{
#ifdef ENABLE_CSHIELD_DEBUG
		ErrorMessage("Thread is suspended");
#endif
		cshieldClientCode = 42;
		CShieldClose();
	}

	// new checks
	DWORD_PTR dwStartAddress = GetThreadStartAddress(myCShieldData.hThread);
	if (myCShieldData.dwStartAddress != dwStartAddress)
	{
#ifdef ENABLE_CSHIELD_DEBUG
		ErrorMessage("Error 2");
#endif
		cshieldClientCode = 42;
		CShieldClose();
	}

	MODULEINFO user32ModInfo = { 0 };
	if (GetModuleInformation(NtCurrentProcess, LoadLibraryA("user32"), &user32ModInfo, sizeof(user32ModInfo)))
	{
		DWORD_PTR dwUser32Low = (DWORD_PTR)user32ModInfo.lpBaseOfDll;
		DWORD_PTR dwUser32Hi = (DWORD_PTR)user32ModInfo.lpBaseOfDll + user32ModInfo.SizeOfImage;

		if (dwUser32Low != myCShieldData.dwUser32Low || dwUser32Hi != myCShieldData.dwUser32Hi)
		{
#ifdef ENABLE_CSHIELD_DEBUG
			ErrorMessage("Error 3");
#endif
			cshieldClientCode = 42;
			CShieldClose();
		}
	}

	MEMORY_BASIC_INFORMATION mbi = { 0 };
	if (VirtualQuery((LPCVOID)dwStartAddress, &mbi, sizeof(mbi)))
	{
		DWORD dwMemType = mbi.Type;
		DWORD_PTR dwMemAllocationBase = (DWORD_PTR)mbi.AllocationBase;

		if (myCShieldData.dwMemType != dwMemType)
		{
#ifdef ENABLE_CSHIELD_DEBUG
			ErrorMessage("Error 4");
#endif
			cshieldClientCode = 42;
			CShieldClose();
		}

		if (myCShieldData.dwMemAllocationBase != dwMemAllocationBase)
		{
#ifdef ENABLE_CSHIELD_DEBUG
			ErrorMessage("Error 5");
#endif
			cshieldClientCode = 42;
			CShieldClose();
		}
	}

	return true;
}

#ifdef ENABLE_CSHIELD_DEBUG
void ErrorMessage(std::string error)
{
	std::ofstream myfile("syserr.txt", std::ios::app);
	if (myfile.is_open())
	{
		myfile << error << "\n";
		myfile.close();
	}
}
#endif
#endif