#include "../../Headers/Hooks/VMTHookManager.h"


namespace BAMEWARE::HOOKS
{
	void VMTHookManager::Initialize(void* instance)
	{
		m_instance = instance;
	}

	void VMTHookManager::Restore()
	{
		for (const auto& f : m_original_functions)
			RestoreVirtualFunction(f.first);
	}

	void* VMTHookManager::HookVirtualFunction(const size_t index, const uintptr_t hooked_func)
	{
		if (m_original_functions.find(index) != m_original_functions.end())
			throw std::runtime_error("Function already hooked!");

		void** vtable = *reinterpret_cast<void***>(m_instance);
		m_original_functions[index] = vtable[index];

		DWORD old_protect;
		VirtualProtect(reinterpret_cast<void*>(vtable + sizeof(void*) * index), sizeof(void*), PAGE_EXECUTE_READWRITE,
		               &old_protect);
		vtable[index] = reinterpret_cast<void*>(hooked_func);
		VirtualProtect(reinterpret_cast<void*>(vtable + sizeof(void*) * index), sizeof(void*), old_protect,
		               &old_protect);

		return m_original_functions.at(index);
	}

	void VMTHookManager::RestoreVirtualFunction(const size_t index)
	{
		if (m_original_functions.find(index) == m_original_functions.end())
			return;

		void** vtable = *reinterpret_cast<void***>(m_instance);

		DWORD old_protect;
		VirtualProtect(reinterpret_cast<void*>(vtable + sizeof(void*) * index), sizeof(void*), PAGE_EXECUTE_READWRITE,
		               &old_protect);
		vtable[index] = m_original_functions.at(index);
		VirtualProtect(reinterpret_cast<void*>(vtable + sizeof(void*) * index), sizeof(void*), old_protect,
		               &old_protect);

		m_original_functions.erase(m_original_functions.find(index));
	}
}
