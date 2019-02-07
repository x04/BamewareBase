#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <ctime>
#include <fstream>
#include <iterator>
#include <comdef.h>
#include <Wbemidl.h>
#include <VersionHelpers.h>
#include <atlstr.h>
#include <wbemidl.h>
#include <sstream>
#include <vector>
#include <psapi.h>

namespace BAMEWARE::PROTECTION::ANTIDEBUG
{
	extern inline BOOL IsDebuggerPresentAPI();
	extern inline BOOL CheckRemoteDebuggerPresentAPI();
	extern inline bool HideThread(HANDLE thread);;
	extern inline BOOL HardwareBreakpoints();
	extern inline BOOL MemoryBreakpoints_PageGuard();
	extern inline BOOL ModuleBoundsHookCheck();
	extern inline BOOL UnhandledExcepFilterTest();
	extern inline BOOL SharedUserData_KernelDebugger();
}