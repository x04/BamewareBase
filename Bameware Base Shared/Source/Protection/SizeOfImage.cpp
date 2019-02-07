#include "../../Headers/Protection/SizeOfImage.h"

#include <Windows.h>
#include <Winternl.h>

namespace BAMEWARE::PROTECTION::ANTIDUMP
{
	void SizeOfImage(int new_size)
	{
#ifdef _WIN64
		const auto peb = PPEB(__readgsqword(0x60));
#else
		const auto peb = PPEB(__readfsdword(0x30));
#endif

		auto table_entry = PLDR_DATA_TABLE_ENTRY(peb->Ldr->InMemoryOrderModuleList.Flink);
		table_entry->DllBase = PVOID(INT_PTR(table_entry->DllBase) + new_size);
	}
}
