#include "../../Headers/Hooks/BreakpointHooks.h"
#include "../../../Bameware Base Shared/Headers/Cryptography/StringEncryption.h"

#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

namespace BAMEWARE::HOOKS
{
	std::vector<BreakpointHookManager::BreakpointHookInfo> BreakpointHookManager::m_hooked_functions;

	bool BreakpointHookManager::Hook(const char* from_module, const char* from_function_name, const char* to_module, const char* to_function_name,
	                                      const BREAKPOINT_CALLING_CONVENTION calling_conventions, const bool overwrite)
	{
		if (m_hooked_functions.size() >= 4)
			throw std::exception(ENCRYPT_STRING("Attempted to hook more than 4 functions with debug registers.")); /// yeah uhm no.

		const auto from_function = GetProcAddress(
			GetModuleHandle(from_module),
			from_function_name);
		const auto to_function = GetProcAddress(
			GetModuleHandle(to_module),
			to_function_name);

		return Hook(*from_function, *to_function, calling_conventions, overwrite);
	}

	bool BreakpointHookManager::Hook(FARPROC from_function, FARPROC to_function, const BREAKPOINT_CALLING_CONVENTION calling_conventions, const bool overwrite)
	{
		if (!from_function || !to_function)
			return false;

		if (m_hooked_functions.size() >= 4)
			throw std::exception(ENCRYPT_STRING("Attempted to hook more than 4 functions with debug registers.")); /// yeah uhm no.

		auto index = SetDebugBreak(from_function);

		if (index == size_t(-1) && !overwrite)
			return false;

		if (index == size_t(-1) && overwrite) /// if overwrite is true then we will attempt to overwrite the current debug register
		{
			if (!ClearDebugBreak(m_hooked_functions.size() + 1))
				return false;

			index = SetDebugBreak(from_function);

			if (index == size_t(-1))
				return false;
		}

		//std::cout << ENCRYPT_STRING("Hooked function at 0x") << std::hex << from_function << ENCRYPT_STRING(" to 0x") << std::hex << to_function << std::endl;

		const auto new_hook = BreakpointHookInfo(from_function, to_function, calling_conventions, index);
		m_hooked_functions.push_back(new_hook);

		return true;
	}
	
	void BreakpointHookManager::Unhook()
	{
		if (m_hooked_functions.empty()) /// Dont try to unhook if we havent hooked anything, retard
			return;

		for (const auto hook : m_hooked_functions)
			ClearDebugBreak(hook.m_register_index);
	}

	size_t BreakpointHookManager::SetDebugBreak(FARPROC address)
	{
		size_t register_index = -1;

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
		THREADENTRY32 entry;
		entry.dwSize = sizeof(THREADENTRY32);

		Thread32First(snapshot, &entry);
		do
		{
			if (entry.th32OwnerProcessID != GetCurrentProcessId())
				continue;

			HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, entry.th32ThreadID);
			CONTEXT ctx;
			ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
			GetThreadContext(thread, &ctx);

#ifdef _WIN64
			/// This is where we actually go through all of the 4 debug registers we can use, if a usable one is found then we will use it
			if (!ctx.Dr0)
			{
				ctx.Dr0 = MakePtr(uint64_t, address);
				ctx.Dr7 |= 0x00000001;
				register_index = 0;
			}
			else if (!ctx.Dr1)
			{
				ctx.Dr1 = MakePtr(uint64_t, address);
				ctx.Dr7 |= 0x00000004;
				register_index = 1;
			}
			else if (!ctx.Dr2)
			{
				ctx.Dr2 = MakePtr(uint64_t, address);
				ctx.Dr7 |= 0x00000010;
				register_index = 2;
			}
			else if (!ctx.Dr3)
			{
				ctx.Dr3 = MakePtr(uint64_t, address);
				ctx.Dr7 |= 0x00000040;
				register_index = 3;
			}
			else /// Failed to find a valid register (all of them are used)
				register_index = -1;
#else
			/// This is where we actually go through all of the 4 debug registers we can use, if a usable one is found then we will use it
			if (!ctx.Dr0)
			{
				ctx.Dr0 = MakePtr(uint32_t, address);
				ctx.Dr7 |= 0x00000001;
				register_index = 0;
			}
			else if (!ctx.Dr1)
			{
				ctx.Dr1 = MakePtr(uint32_t, address);
				ctx.Dr7 |= 0x00000004;
				register_index = 1;
			}
			else if (!ctx.Dr2)
			{
				ctx.Dr2 = MakePtr(uint32_t, address);
				ctx.Dr7 |= 0x00000010;
				register_index = 2;
			}
			else if (!ctx.Dr3)
			{
				ctx.Dr3 = MakePtr(uint32_t, address);
				ctx.Dr7 |= 0x00000040;
				register_index = 3;
			}
			else /// Failed to find a valid register (all of them are used)
				register_index = -1;
#endif

			ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
			SetThreadContext(thread, &ctx);
			CloseHandle(thread);
		} while (Thread32Next(snapshot, &entry));

		return register_index;
	}

	bool BreakpointHookManager::ClearDebugBreak(const size_t index)
	{
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
		THREADENTRY32 entry;
		entry.dwSize = sizeof(THREADENTRY32);
		Thread32First(snapshot, &entry);

		do
		{
			if (entry.th32OwnerProcessID != GetCurrentProcessId())
				continue;

			HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, entry.th32ThreadID);
			CONTEXT ctx;
			ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
			GetThreadContext(thread, &ctx);

			/// Find the debug register at the given index and reset it (unhook)
			switch (index)
			{
			case 0:
				ctx.Dr0 = 0;
				break;
			case 1:
				ctx.Dr1 = 0;
				ctx.Dr7 &= 0xFF0FFFFB;
				break;
			case 2:
				ctx.Dr2 = 0;
				ctx.Dr7 &= 0xF0FFFFEF;
				break;
			case 3:
				ctx.Dr3 = 0;
				ctx.Dr7 &= 0xFFFFFBF;
				break;
			default:
				return false;
			}

			SetThreadContext(thread, &ctx);
			CloseHandle(thread);
		} while (Thread32Next(snapshot, &entry));

		return true;
	}

	LONG CALLBACK BreakpointHookManager::ExceptionHandler(const PEXCEPTION_POINTERS ex)
	{
#ifdef _WIN64
		if (ex->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP)
			return EXCEPTION_CONTINUE_SEARCH;

		BreakpointHookInfo info = BreakpointHookInfo();
		for (const auto func : m_hooked_functions)
			if (ex->ContextRecord->Rip == MakePtr(uint64_t, func.m_from_function, 0))
			{
				info = func;
				break;
			}

		/// Ensure the exception is hooked by us
		if (info.m_register_index == size_t(-1))
			return EXCEPTION_CONTINUE_SEARCH;

		/// Redirect to our function
		ex->ContextRecord->Rip = MakePtr(uint64_t, info.m_to_function, 0);

		//std::cout << ENCRYPT_STRING("Call to index ") << int(info.m_register_index) << ENCRYPT_STRING(" hooked to function ") << std::hex << info.m_from_function
		//				<< ENCRYPT_STRING(" redirected to ") << std::hex << info.m_to_function << std::endl;

		return EXCEPTION_CONTINUE_EXECUTION;
#else
		if (ex->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP)
			return EXCEPTION_CONTINUE_SEARCH;

		DebugHookInfo info = DebugHookInfo();
		for (const auto func : m_hooked_functions)
			if (ex->ContextRecord->Eip == MakePtr(uint32_t, func.m_from_function, 0))
			{
				info = func;
				break;
			}

		/// Ensure the exception is hooked by us
		if (info.m_register_index == size_t(-1))
			return EXCEPTION_CONTINUE_SEARCH;

		/// Redirect to our function
		ex->ContextRecord->Eip = MakePtr(uint32_t, info.m_to_function, 0);
		return EXCEPTION_CONTINUE_EXECUTION;
#endif
	}
}
