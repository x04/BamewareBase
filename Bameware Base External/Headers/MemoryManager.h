#pragma once

#include <cstdint>
#include <Windows.h>
#include <string>
#include <vector>


namespace BAMEWARE
{
	/// 32bit: MemoryManager<uint32_t>
	/// 64bit: MemoryManager<uint64_t>
	template <typename ptr>
	class MemoryManager
	{
	public:
		MemoryManager() = default;
		MemoryManager(uint32_t process_id) { Initialize(process_id); }

	public:
		struct ModuleInfo
		{
			std::string m_module_name,
				m_module_path;
			ptr m_base_address;
		};
		struct ProcessInfo
		{
			uint32_t m_process_id;
			std::string m_process_name;
			std::vector<ModuleInfo> m_modules;
		};

	public:
		bool Initialize(uint32_t process_id);
		void LoadModules();
		void Release();

		ptr GetModuleAddress(const std::string& module_name) const;
		HWND GetAssociatedWindow() const;
		HANDLE GetProcessHandle() const { return m_process_handle; }
		uint32_t GetProcessID() const { return m_process_id; }

		static bool ReadMemoryEx(HANDLE process_handle, ptr address, void* buffer, size_t size);
		static bool WriteMemoryEx(HANDLE process_handle, ptr address, void* buffer, size_t size);

		template <typename T>
		T ReadMemory(ptr address, ptr offset = ptr(0)) const
		{
			T value;
			ReadMemoryEx(m_process_handle, address + offset, &value, sizeof(T));
			return value;
		}
		template <typename T>
		void WriteMemory(T value, ptr address, ptr offset = ptr(0)) const
		{
			WriteMemoryEx(m_process_handle, address + offset, &value, sizeof(T));
		}

	public:
		static std::vector<ModuleInfo> GetModules(uint32_t process_id);
		static std::vector<ProcessInfo> GetProcesses(const std::string& substr = std::string(""));

	private:
		uint32_t m_process_id;
		HANDLE m_process_handle = nullptr;
		std::vector<ModuleInfo> m_modules;
	};
}
