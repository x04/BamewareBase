#pragma once
#include <Windows.h>

namespace BAMEWARE::HOOKS
{
	uintptr_t IATHookFunctionInternal(const char* func, void* new_func, HMODULE module = nullptr);
	uintptr_t EATHookFunctionInternal(const char* func, void* new_func, HMODULE module = nullptr);
}
