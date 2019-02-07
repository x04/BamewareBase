#pragma once

#include "../MemoryManager.h"

namespace BAMEWARE::HOOKS
{
	template <typename ptr>
	ptr EATHookFunctionExternal(ptr hooked_function, MemoryManager<ptr>& memory_manager, const char* module_name, const char* function_name);

	template <typename ptr>
	ptr IATHookFunctionExternal(ptr hooked_function, MemoryManager<ptr>& memory_manager, const char* module_name, const char* function_name);
}
