#include "../Headers/MemoryManager.h"

#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>


namespace BAMEWARE
{
	DWORD process_id_to_match = 0; HWND matching_window = nullptr;
	int CALLBACK EnumWindowsProc(HWND hwnd, LPARAM)
	{
		DWORD process_id = 0;
		GetWindowThreadProcessId(hwnd, &process_id);

		if (process_id != process_id_to_match)
			return 1;

		matching_window = hwnd;
		return 0;
	}

	/* 32bit */
	bool MemoryManager<uint32_t>::Initialize(const uint32_t process_id)
	{
		m_process_id = process_id;
		m_process_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, m_process_id);

		LoadModules();

		return m_process_handle;
	}
	void MemoryManager<uint32_t>::Release()
	{
		if (m_process_handle)
			CloseHandle(m_process_handle);

		m_process_handle = nullptr;
	}

	template<>
	void MemoryManager<uint32_t>::LoadModules()
	{
		m_modules = GetModules(m_process_id);
	}

	HWND MemoryManager<uint32_t>::GetAssociatedWindow() const
	{
		process_id_to_match = m_process_id;
		matching_window = nullptr;

		EnumWindows(EnumWindowsProc, NULL);
		return matching_window;
	}
	uint32_t MemoryManager<uint32_t>::GetModuleAddress(const std::string& module_name) const
	{
		for (const auto& mod : m_modules)
		{
			if (mod.m_module_name == module_name)
				return mod.m_base_address;
		}

		return 0;
	}

	bool MemoryManager<uint32_t>::ReadMemoryEx(HANDLE process_handle, const uint32_t address, void* buffer, const size_t size)
	{
		SIZE_T tmp;

#ifdef _WIN64
		return !ReadProcessMemory(process_handle, reinterpret_cast<void*>(uint64_t(address)), buffer, size, &tmp);
#else
		return !ReadProcessMemory(process_handle, reinterpret_cast<void*>(address), buffer, size, &tmp);
#endif
	}
	bool MemoryManager<uint32_t>::WriteMemoryEx(HANDLE process_handle, const uint32_t address, void* buffer, const size_t size)
	{
		SIZE_T tmp;

#ifdef _WIN64
		return !WriteProcessMemory(process_handle, reinterpret_cast<void*>(uint64_t(address)), buffer, size, &tmp);
#else
		return !WriteProcessMemory(process_handle, reinterpret_cast<void*>(address), buffer, size, &tmp);
#endif
	}

	template <>
	std::vector<MemoryManager<uint32_t>::ModuleInfo> MemoryManager<uint32_t>::GetModules(const uint32_t process_id)
	{
		std::vector<ModuleInfo> modules;

		const auto process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
		if (!process_handle || process_handle == INVALID_HANDLE_VALUE)
			return modules;

		HMODULE module_handles[1024];
		if (DWORD bytes_needed;  EnumProcessModules(process_handle, module_handles, sizeof(module_handles), &bytes_needed))
		{
			for (unsigned int i = 0; i < (bytes_needed / sizeof(HMODULE)); i++)
			{
				TCHAR module_path[MAX_PATH];

				if (GetModuleFileNameEx(process_handle, module_handles[i], module_path,
					sizeof(module_path) / sizeof(TCHAR)))
				{
					ModuleInfo info;

					info.m_module_name = module_path;

					/// chop off the beggining of the module path
					for (size_t x = info.m_module_name.size() - 1; x >= 0; x--)
					{
						if (info.m_module_name[x] == '\\')
						{
							info.m_module_name.erase(info.m_module_name.begin(), info.m_module_name.begin() + x + 1);
							break;
						}
					}

					info.m_module_path = module_path;

#ifdef _WIN64
					info.m_base_address = uint32_t(uint64_t(module_handles[i]));
#else
					info.m_base_address = uint32_t(module_handles[i]);
#endif

					modules.push_back(info);
			}
		}
	}

		CloseHandle(process_handle);
		return modules;
	}
	std::vector<MemoryManager<uint32_t>::ProcessInfo> MemoryManager<uint32_t>::GetProcesses(const std::string& substr)
	{
		std::vector<ProcessInfo> processes;

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);

		const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (snapshot == INVALID_HANDLE_VALUE)
			return processes;

		while (Process32Next(snapshot, &entry))
		{
			if (!strstr(entry.szExeFile, substr.c_str()))
				continue;

			ProcessInfo info{};

			info.m_process_id = entry.th32ProcessID;
			info.m_process_name = entry.szExeFile;
			info.m_modules = GetModules(entry.th32ProcessID);

			processes.push_back(info);
		}
		CloseHandle(snapshot);

		return processes;
	}
		
	/* 64bit */
	bool MemoryManager<uint64_t>::Initialize(const uint32_t process_id)
	{
		m_process_id = process_id;
		m_process_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, m_process_id);

		LoadModules();

		return m_process_handle;
	}
	void MemoryManager<uint64_t>::Release()
	{
		if (m_process_handle)
			CloseHandle(m_process_handle);

		m_process_handle = nullptr;
	}

	template <>
	void MemoryManager<uint64_t>::LoadModules()
	{
		m_modules = GetModules(m_process_id);
	}

	HWND MemoryManager<uint64_t>::GetAssociatedWindow() const
	{
		process_id_to_match = m_process_id;
		matching_window = nullptr;

		EnumWindows(EnumWindowsProc, NULL);
		return matching_window;
	}
	uint64_t MemoryManager<uint64_t>::GetModuleAddress(const std::string& module_name) const
	{
		for (const auto& mod : m_modules)
		{
			if (mod.m_module_name == module_name)
				return mod.m_base_address;
		}

		return 0;
	}

	bool MemoryManager<uint64_t>::ReadMemoryEx(HANDLE process_handle, const uint64_t address, void* buffer, const size_t size)
	{
		SIZE_T tmp;
		return !ReadProcessMemory(process_handle, reinterpret_cast<void*>(address), buffer, size, &tmp);
	}
	bool MemoryManager<uint64_t>::WriteMemoryEx(HANDLE process_handle, const uint64_t address, void* buffer, const size_t size)
	{
		SIZE_T tmp;
		return !WriteProcessMemory(process_handle, reinterpret_cast<void*>(address), buffer, size, &tmp);
	}

	template <>
	std::vector<MemoryManager<uint64_t>::ModuleInfo> MemoryManager<uint64_t>::GetModules(const uint32_t process_id)
	{
		std::vector<ModuleInfo> modules;

		const auto process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
		if (!process_handle || process_handle == INVALID_HANDLE_VALUE)
			return modules;

		HMODULE module_handles[1024];
		if (DWORD bytes_needed;  EnumProcessModules(process_handle, module_handles, sizeof(module_handles), &bytes_needed))
		{
			for (unsigned int i = 0; i < (bytes_needed / sizeof(HMODULE)); i++)
			{
				TCHAR module_path[MAX_PATH];

				if (GetModuleFileNameEx(process_handle, module_handles[i], module_path,
					sizeof(module_path) / sizeof(TCHAR)))
				{
					ModuleInfo info;

					info.m_module_name = module_path;

					/// chop off the beggining of the module path
					for (size_t x = info.m_module_name.size() - 1; x >= 0; x--)
					{
						if (info.m_module_name[x] == '\\')
						{
							info.m_module_name.erase(info.m_module_name.begin(), info.m_module_name.begin() + x + 1);
							break;
						}
					}

					info.m_module_path = module_path;
					info.m_base_address = uint64_t(module_handles[i]);

					modules.push_back(info);
				}
			}
		}

		CloseHandle(process_handle);
		return modules;
	}
	std::vector<MemoryManager<uint64_t>::ProcessInfo> MemoryManager<uint64_t>::GetProcesses(const std::string& substr)
	{
		std::vector<ProcessInfo> processes;

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(entry);

		const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (snapshot == INVALID_HANDLE_VALUE)
			return processes;

		while (Process32Next(snapshot, &entry))
		{
			if (!strstr(entry.szExeFile, substr.c_str()))
				continue;

			ProcessInfo info{};

			info.m_process_id = entry.th32ProcessID;
			info.m_process_name = entry.szExeFile;
			info.m_modules = GetModules(entry.th32ProcessID);

			processes.push_back(info);
		}
		CloseHandle(snapshot);

		return processes;
	}
}
