#pragma once

#include <unordered_map>
#include <Windows.h>


namespace BAMEWARE::HOOKS
{
	class VMTHookManager
	{
	public:
		VMTHookManager() = default;
		VMTHookManager(void* instance) { Initialize(instance); }
		~VMTHookManager() { Restore(); }

	public:
		void Initialize(void* instance);
		void Restore();

		template <typename T>
		static T GetVirtualFunction(void* instance, const size_t index)
		{
			void** vtable = *reinterpret_cast<void***>(instance);
			return reinterpret_cast<T>(vtable[index]);
		}

		template <typename T>
		T HookVirtualFunction(const size_t index, void* hooked_func)
		{
			return reinterpret_cast<T>(HookVirtualFunction(index, uintptr_t(hooked_func)));
		}
		void RestoreVirtualFunction(size_t index);

	private:
		void* HookVirtualFunction(size_t index, uintptr_t hooked_func);

	private:
		/// <index, original_ptr>
		std::unordered_map<size_t, void*> m_original_functions;
		void* m_instance = nullptr;
	};
}