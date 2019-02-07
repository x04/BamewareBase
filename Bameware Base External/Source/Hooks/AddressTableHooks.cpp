#include "../../Headers/Hooks/AddressTableHooks.h"

#include <string>
#include <Windows.h>

#include "../../Headers/MemoryManager.h"
#include "../../Headers/ParsedPEHeader.h"


namespace BAMEWARE::HOOKS
{
	/* 32-bit */
	uint32_t EATHookFunction(const uint32_t hooked_function, MemoryManager<uint32_t>& memory_manager, const char* module_name, const char* function_name)
	{
		ParsedPEHeader<uint32_t> pe_header{};
		if (!pe_header.Load(memory_manager.GetProcessID(), module_name))
			return 0;

		for (size_t i = 0; i < pe_header.m_parsed_eat.m_num_functions; i++)
		{
			if (strcmp(function_name, pe_header.m_parsed_eat.m_functions[i].m_name) != 0)
				continue;

			unsigned long old_protect;
			VirtualProtectEx(memory_manager.GetProcessHandle(), reinterpret_cast<void*>(pe_header.m_parsed_eat.m_functions[i].m_address_address), 4, PAGE_EXECUTE_READWRITE, &old_protect);
			memory_manager.WriteMemory<uint32_t>(hooked_function - pe_header.m_module_base_addr, pe_header.m_parsed_eat.m_functions[i].m_address_address);
			VirtualProtectEx(memory_manager.GetProcessHandle(), reinterpret_cast<void*>(pe_header.m_parsed_eat.m_functions[i].m_address_address), 4, old_protect, &old_protect);

			const auto original_addr = pe_header.m_parsed_eat.m_functions[i].m_address;
			pe_header.Release();
			return original_addr;
		}

		pe_header.Release();
		return 0;
	}
	uint32_t IATHookFunction(const uint32_t hooked_function, MemoryManager<uint32_t>& memory_manager, const char* module_name, const char* function_name)
	{
		ParsedPEHeader<uint32_t> pe_header{};
		if (!pe_header.Load(memory_manager.GetProcessID(), module_name))
			return 0;

		for (size_t i = 0; i < pe_header.m_parsed_iat.m_num_entries; i++)
		{
			if (strcmp(module_name, pe_header.m_parsed_iat.m_entries[i].m_name) != 0)
				continue;

			for (size_t x = 0; x < pe_header.m_parsed_iat.m_entries[i].m_num_functions; x++)
			{
				if (strcmp(function_name, pe_header.m_parsed_iat.m_entries[i].m_functions[x].m_name) != 0)
					continue;

				unsigned long old_protection;
				VirtualProtectEx(memory_manager.GetProcessHandle(), reinterpret_cast<void*>(pe_header.m_parsed_iat.m_entries[i].m_functions[x].m_address_address), 4, PAGE_EXECUTE_READWRITE, &old_protection);
				memory_manager.WriteMemory<uint32_t>(hooked_function, pe_header.m_parsed_iat.m_entries[i].m_functions[x].m_address_address);
				VirtualProtectEx(memory_manager.GetProcessHandle(), reinterpret_cast<void*>(pe_header.m_parsed_iat.m_entries[i].m_functions[x].m_address_address), 4, old_protection, &old_protection);

				const auto original_addr = pe_header.m_parsed_iat.m_entries[i].m_functions[x].m_address;
				pe_header.Release();
				return original_addr;
			}
		}

		pe_header.Release();
		return 0;
	}

	/* 64-bit */
	uint64_t EATHookFunction(const uint64_t hooked_function, MemoryManager<uint64_t>& memory_manager, const char* module_name, const char* function_name)
	{
		ParsedPEHeader<uint64_t> pe_header{};
		if (!pe_header.Load(memory_manager.GetProcessID(), module_name))
			return 0;

		for (size_t i = 0; i < pe_header.m_parsed_eat.m_num_functions; i++)
		{
			if (strcmp(function_name, pe_header.m_parsed_eat.m_functions[i].m_name) != 0)
				continue;

			unsigned long old_protect;
			VirtualProtectEx(memory_manager.GetProcessHandle(), reinterpret_cast<void*>(pe_header.m_parsed_eat.m_functions[i].m_address_address), 4, PAGE_EXECUTE_READWRITE, &old_protect);
			memory_manager.WriteMemory<uint64_t>(hooked_function - pe_header.m_module_base_addr, pe_header.m_parsed_eat.m_functions[i].m_address_address);
			VirtualProtectEx(memory_manager.GetProcessHandle(), reinterpret_cast<void*>(pe_header.m_parsed_eat.m_functions[i].m_address_address), 4, old_protect, &old_protect);

			const auto original_addr = pe_header.m_parsed_eat.m_functions[i].m_address;
			pe_header.Release();
			return original_addr;
		}

		pe_header.Release();
		return 0;
	}
	uint64_t IATHookFunction(const uint64_t hooked_function, MemoryManager<uint64_t>& memory_manager, const char* module_name, const char* function_name)
	{
		ParsedPEHeader<uint64_t> pe_header{};
		if (!pe_header.Load(memory_manager.GetProcessID(), module_name))
			return 0;

		for (size_t i = 0; i < pe_header.m_parsed_iat.m_num_entries; i++)
		{
			if (strcmp(module_name, pe_header.m_parsed_iat.m_entries[i].m_name) != 0)
				continue;

			for (size_t x = 0; x < pe_header.m_parsed_iat.m_entries[i].m_num_functions; x++)
			{
				if (strcmp(function_name, pe_header.m_parsed_iat.m_entries[i].m_functions[x].m_name) != 0)
					continue;

				unsigned long old_protection;
				VirtualProtectEx(memory_manager.GetProcessHandle(), reinterpret_cast<void*>(pe_header.m_parsed_iat.m_entries[i].m_functions[x].m_address_address), 4, PAGE_EXECUTE_READWRITE, &old_protection);
				memory_manager.WriteMemory<uint64_t>(hooked_function, pe_header.m_parsed_iat.m_entries[i].m_functions[x].m_address_address);
				VirtualProtectEx(memory_manager.GetProcessHandle(), reinterpret_cast<void*>(pe_header.m_parsed_iat.m_entries[i].m_functions[x].m_address_address), 4, old_protection, &old_protection);

				const auto original_addr = pe_header.m_parsed_iat.m_entries[i].m_functions[x].m_address;
				pe_header.Release();
				return original_addr;
			}
		}

		pe_header.Release();
		return 0;
	}
}
