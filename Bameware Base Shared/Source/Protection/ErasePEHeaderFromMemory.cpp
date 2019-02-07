#include "../../Headers/Protection/ErasePEHeaderFromMeory.h"

namespace BAMEWARE::PROTECTION::ANTIDUMP
{
	void ErasePEHeaderFromMemory(const HMODULE module)
	{
		DWORD old_protect = 0;

		// Get base address of module
		auto* base_addr = (char*)module;

		// Change memory protection
		VirtualProtect(base_addr, 4096, // Assume x86 page size
		               PAGE_READWRITE, &old_protect);

		// Erase the header
		SecureZeroMemory(base_addr, 4096);
	}
}
