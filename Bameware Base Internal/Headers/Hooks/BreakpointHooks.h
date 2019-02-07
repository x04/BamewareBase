#pragma once

#include <Windows.h>
#include <vector>

#define MakePtr( cast, ptr ) (cast)( (DWORD_PTR)(ptr) ) 

namespace BAMEWARE::HOOKS
{
	enum BREAKPOINT_CALLING_CONVENTION
	{
		NORMAL,	  /// Includes stdcall and cdecl
		FASTCALL,
		NO_RET	  /// Undocumented but not unused, we will probably never use it. But it's here if we ever do.
	};

	class BreakpointHookManager
	{
	public:
		BreakpointHookManager() { AddVectoredExceptionHandler(0, ExceptionHandler); } /// when the manager is created be sure to intialize the exception handler
		~BreakpointHookManager() { Unhook(); } /// if the manager gets destroyed we want to be sure to remove our hooks.

		static bool Hook(const char* from_module, const char* from_function_name, const char* to_module, const char* to_function_name,
			BREAKPOINT_CALLING_CONVENTION calling_conventions = NORMAL, bool overwrite = false);
		static bool Hook(FARPROC from_function, FARPROC to_function, BREAKPOINT_CALLING_CONVENTION calling_conventions = NORMAL, bool overwrite = false);
		static void Unhook();

	private:

		static size_t SetDebugBreak(FARPROC address);
		static bool ClearDebugBreak(size_t index);

		static LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ex);

	private:
		struct BreakpointHookInfo
		{
			BreakpointHookInfo()
			{
				m_register_index = -1;
				m_from_function = nullptr;
				m_to_function = nullptr;
				m_calling_conventions = NORMAL;
			}
			BreakpointHookInfo(FARPROC from_func, FARPROC to_func, const size_t index)
			{
				m_register_index = index;
				m_from_function = from_func;
				m_to_function = to_func;
				m_calling_conventions = NORMAL;
			}
			BreakpointHookInfo(FARPROC from_func, FARPROC to_func, const BREAKPOINT_CALLING_CONVENTION cc, const size_t index)
			{
				m_register_index = index;
				m_from_function = from_func;
				m_to_function = to_func;
				m_calling_conventions = cc;
			}

			size_t m_register_index;
			FARPROC m_from_function;
			FARPROC m_to_function;
			BREAKPOINT_CALLING_CONVENTION m_calling_conventions;
		};

	private:
		static std::vector<BreakpointHookInfo> m_hooked_functions;
	};
}
