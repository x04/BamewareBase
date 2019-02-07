#include <Windows.h>
#include <Winternl.h>

namespace BAMEWARE::PROTECTION::ANTIDUMP
{
	typedef struct _PEB_LDR_DATA_
	{
		ULONG Length;
		BOOLEAN Initialized;
		PVOID SsHandle;
		LIST_ENTRY InLoadOrderModuleList;
		LIST_ENTRY InMemoryOrderModuleList;
		LIST_ENTRY InInitializationOrderModuleList;
	} PEB_LDR_DATA_, *PPEB_LDR_DATA_;

	typedef struct _LDR_MODULE
	{
		LIST_ENTRY InLoadOrderModuleList;
		LIST_ENTRY InMemoryOrderModuleList;
		LIST_ENTRY InInitializationOrderModuleList;
		PVOID BaseAddress;
		PVOID EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING FullDllName;
		UNICODE_STRING BaseDllName;
		ULONG Flags;
		SHORT LoadCount;
		SHORT TlsIndex;
		LIST_ENTRY HashTableEntry;
		ULONG TimeDateStamp;
	} LDR_MODULE, *PLDR_MODULE;

	void UnlinkModule(char* module)
	{
		DWORD peb = 0, offset = 0;
		PLIST_ENTRY user_module_head, user_module;
		PPEB_LDR_DATA_ ldr_data;
		PLDR_MODULE ldr_module = nullptr;
		PUNICODE_STRING p_module = nullptr;
		char module_name[512];
		int i = 0, n = 0;
#ifndef _WIN64
		_asm
		{
			pushad
			mov eax, fs: [48]
			mov peb, eax
			popad
		}

		ldr_data = PPEB_LDR_DATA_(PDWORD(*PDWORD(peb + 12)));
#else
		BYTE* teb = reinterpret_cast<BYTE*>(__readgsqword(0x30));
		ldr_data = PPEB_LDR_DATA_(PULONGLONG(*PULONGLONG((*PULONGLONG(teb + 0x60)) + 0x18)));
#endif

		for (; i < 3; i++)
		{
			switch (i)
			{
			case 0:
				user_module_head = user_module = PLIST_ENTRY(&(ldr_data->InLoadOrderModuleList));
				offset = 0;
				break;

			case 1:
				user_module_head = user_module = PLIST_ENTRY(&(ldr_data->InMemoryOrderModuleList));
#ifndef _WIN64
				offset = 8;
#else
				offset = 16;
#endif
				break;
			case 2:
				user_module_head = user_module = PLIST_ENTRY(&(ldr_data->InInitializationOrderModuleList));
#ifndef _WIN64
				offset = 16;
#else
				offset = 32;
#endif
				break;
			}

			while (user_module->Flink != user_module_head)
			{
				user_module = user_module->Flink;
#ifndef _WIN64
				p_module = PUNICODE_STRING(DWORD(user_module) + (36 - offset));
#else
				p_module = PUNICODE_STRING(LONGLONG(user_module) + (72 - offset));
#endif

				for (n = 0; n < (p_module->Length) / 2 && n < 512; n++)
					module_name[n] = CHAR(*((p_module->Buffer) + (n)));

				module_name[n] = '\0';
				if (strstr(module_name, module))
				{
#ifndef _WIN64
					if (!ldr_module)
						ldr_module = PLDR_MODULE(DWORD(user_module) - offset);
#else
					if (!ldr_module)
						ldr_module = PLDR_MODULE(LONGLONG(user_module) - offset);
#endif
					user_module->Blink->Flink = user_module->Flink;
					user_module->Flink->Blink = user_module->Blink;
				}
			}
		}

		/// Unlink from LdrpHashTable
		if (ldr_module)
		{
			ldr_module->HashTableEntry.Blink->Flink = ldr_module->HashTableEntry.Flink;
			ldr_module->HashTableEntry.Flink->Blink = ldr_module->HashTableEntry.Blink;
		}
	}
}
