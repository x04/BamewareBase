#include <iostream>
#include <windows.h>
#include <cstdint>

namespace BAMEWARE::HOOKS
{
	uintptr_t IATHookFunctionInternal(const char* func, void* new_func, HMODULE module = nullptr)
	{
		const auto find_func = [](const char* func, HMODULE module = nullptr) -> void**
		{
			if (!module)
				module = GetModuleHandle(NULL);

			const auto img_dos_headers = PIMAGE_DOS_HEADER(module);
			const auto img_nt_headers = PIMAGE_NT_HEADERS((byte*)img_dos_headers + img_dos_headers->e_lfanew);
			const auto img_import_desc = PIMAGE_IMPORT_DESCRIPTOR(
				(byte*)img_dos_headers + img_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

			for (IMAGE_IMPORT_DESCRIPTOR *iid = img_import_desc; iid->Name != 0; iid++) {
				for (int func_idx = 0; *(func_idx + (void**)(iid->FirstThunk + size_t(module))) != nullptr; func_idx++) {
					auto mod_func_name = (char*)(*(func_idx + (size_t*)(iid->OriginalFirstThunk + size_t(module))) + size_t(module) + 2);
					const auto nmod_func_name = intptr_t(mod_func_name);
					if (nmod_func_name >= 0) {
						if (!strcmp(func, mod_func_name))
							return func_idx + (void**)(iid->FirstThunk + size_t(module));
					}
				}
			}

			return 0;
		};

		auto&& func_ptr = find_func(func, module);
		if (*func_ptr == new_func || *func_ptr == nullptr)
			return 0;

		DWORD old_rights, new_rights = PAGE_READWRITE;
		VirtualProtect(func_ptr, sizeof(uintptr_t), new_rights, &old_rights);
		auto ret = uintptr_t(*func_ptr);
		*func_ptr = new_func;
		VirtualProtect(func_ptr, sizeof(uintptr_t), old_rights, &new_rights);

		return 0;
	}

	uintptr_t EATHookFunctionInternal(const char* func, void* new_func, HMODULE module = nullptr)
	{
		const auto find_func = [](const char* func, HMODULE module = nullptr) -> void**
		{
			if (!module)
				module = GetModuleHandle(NULL);

			const auto img_dos_headers = PIMAGE_DOS_HEADER(module);
			const auto img_nt_headers = PIMAGE_NT_HEADERS((byte*)img_dos_headers + img_dos_headers->e_lfanew);
			const auto img_import_desc = PIMAGE_IMPORT_DESCRIPTOR(
				(byte*)img_dos_headers + img_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

			for (IMAGE_IMPORT_DESCRIPTOR *iid = img_import_desc; iid->Name != 0; iid++) {
				for (int func_idx = 0; *(func_idx + (void**)(iid->FirstThunk + size_t(module))) != nullptr; func_idx++) {
					auto mod_func_name = (char*)(*(func_idx + (size_t*)(iid->OriginalFirstThunk + size_t(module))) + size_t(module) + 2);
					const auto nmod_func_name = intptr_t(mod_func_name);
					if (nmod_func_name >= 0) {
						if (!strcmp(func, mod_func_name))
							return func_idx + (void**)(iid->FirstThunk + size_t(module));
					}
				}
			}

			return 0;
		};

		auto&& func_ptr = find_func(func, module);
		if (*func_ptr == new_func || *func_ptr == nullptr)
			return 0;

		DWORD old_rights, new_rights = PAGE_READWRITE;
		VirtualProtect(func_ptr, sizeof(uintptr_t), new_rights, &old_rights);
		auto ret = uintptr_t(*func_ptr);
		*func_ptr = new_func;
		VirtualProtect(func_ptr, sizeof(uintptr_t), old_rights, &new_rights);

		return 0;
	}
}