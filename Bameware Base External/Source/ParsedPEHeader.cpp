#include "../Headers/ParsedPEHeader.h"

#include "../Headers/MemoryManager.h"

#include <winnt.h>


namespace BAMEWARE
{
	/* x64 */
	bool ParsedPEHeader<uint64_t>::Load(const uint32_t process_id, const char* module_name)
	{
		MemoryManager<uint64_t> mem_manager;
		if (!mem_manager.Initialize(process_id))
			return false;

		m_module_base_addr = mem_manager.GetModuleAddress(module_name);
		if (!m_module_base_addr)
			return false;

		const auto dos_header = mem_manager.ReadMemory<IMAGE_DOS_HEADER>(m_module_base_addr);
		const auto nt_header = mem_manager.ReadMemory<IMAGE_NT_HEADERS>(m_module_base_addr, dos_header.e_lfanew);
		
		/// make sure the signature of the header is "PE"
		{ 
			char tmp[4];
			*reinterpret_cast<uint32_t*>(tmp) = nt_header.Signature;
			if (tmp[0] != 'P' || tmp[1] != 'E')
				return false;
		}

		const auto& file_header = nt_header.FileHeader;
		const auto& optional_header = nt_header.OptionalHeader;

		/// get the flippen process type
		m_process_type = PROCESS_TYPE_OTHER;
		if (file_header.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)
			m_process_type = PROCESS_TYPE_EXE;
		else if (file_header.Characteristics & IMAGE_FILE_DLL)
			m_process_type = PROCESS_TYPE_DLL;

		m_num_sections = file_header.NumberOfSections;
		m_time_stamp = file_header.TimeDateStamp;

		/// subsystem
		switch (optional_header.Subsystem)
		{
		case IMAGE_SUBSYSTEM_NATIVE:
			m_process_subsystem = PROCESS_SUBSYSTEM_DRIVER;
			break;
		case IMAGE_SUBSYSTEM_WINDOWS_CUI:
			m_process_subsystem = PROCESS_SUBSYSTEM_CONSOLE;
			break;
		case IMAGE_SUBSYSTEM_WINDOWS_GUI:
			m_process_subsystem = PROCESS_SUBSYSTEM_GUI;
			break;
		default:
			m_process_subsystem = PROCESS_SUBSYSTEM_OTHER;
		}

		/// import and export address table
		m_parsed_eat.m_base_rva = optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		m_parsed_eat.m_size = optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

		m_parsed_iat.m_base_rva = optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		m_parsed_iat.m_size = optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		m_parsed_iat.m_num_entries = size_t((m_parsed_iat.m_size / sizeof(IMAGE_IMPORT_DESCRIPTOR)) - 1);
		m_parsed_iat.m_entries = reinterpret_cast<ParsedIAT::IATEntry*>(malloc(m_parsed_iat.m_num_entries * sizeof(ParsedIAT::IATEntry)));

		/// go through each IAT entry
		for (size_t i = 0; i < m_parsed_iat.m_num_entries; i++)
		{
			const auto iat_entry = mem_manager.ReadMemory<IMAGE_IMPORT_DESCRIPTOR>(m_module_base_addr,
				m_parsed_iat.m_base_rva + (i * sizeof(IMAGE_IMPORT_DESCRIPTOR)));

			/// copy the name
			{
				size_t letter_index = 0;
				m_parsed_iat.m_entries[i].m_name[0] = mem_manager.ReadMemory<char>(m_module_base_addr, iat_entry.Name);
				while (m_parsed_iat.m_entries[i].m_name[letter_index] && letter_index < 255)
				{
					letter_index++;
					m_parsed_iat.m_entries[i].m_name[letter_index] = mem_manager.ReadMemory<char>(m_module_base_addr, iat_entry.Name + letter_index);
				}
				m_parsed_iat.m_entries[i].m_name[letter_index] = '\0';
			}

			/// get the number of imported functions the entry contains 
			auto thunk_entry = mem_manager.ReadMemory<IMAGE_THUNK_DATA>(m_module_base_addr, iat_entry.FirstThunk);
			for (m_parsed_iat.m_entries[i].m_num_functions = 0; thunk_entry.u1.AddressOfData != 0;)
			{
				m_parsed_iat.m_entries[i].m_num_functions++;
				thunk_entry = mem_manager.ReadMemory<IMAGE_THUNK_DATA>(m_module_base_addr, iat_entry.FirstThunk + (m_parsed_iat.m_entries[i].m_num_functions * sizeof(IMAGE_THUNK_DATA)));
			}

			m_parsed_iat.m_entries[i].m_functions = reinterpret_cast<ParsedIAT::IATEntry::IATFunction*>(malloc(m_parsed_iat.m_entries[i].m_num_functions * sizeof(ParsedIAT::IATEntry::IATFunction)));

			/// iterate through every thunk and gather info :sunglasses:
			for (size_t x = 0; x < m_parsed_iat.m_entries[i].m_num_functions; x++)
			{
				thunk_entry = mem_manager.ReadMemory<IMAGE_THUNK_DATA>(m_module_base_addr, iat_entry.FirstThunk + (x * sizeof(IMAGE_THUNK_DATA)));
				m_parsed_iat.m_entries[i].m_functions[x].m_address_address = m_module_base_addr + iat_entry.FirstThunk + (x * sizeof(IMAGE_THUNK_DATA));
				const auto original_thunk_entry = mem_manager.ReadMemory<IMAGE_THUNK_DATA>(m_module_base_addr, iat_entry.OriginalFirstThunk + (x * sizeof(IMAGE_THUNK_DATA)));
				auto import_by_name = mem_manager.ReadMemory<IMAGE_IMPORT_BY_NAME>(m_module_base_addr, original_thunk_entry.u1.AddressOfData);

				/// get the name of the function
				{
					size_t letter_index = 0;
					m_parsed_iat.m_entries[i].m_functions[x].m_name[0] = mem_manager.ReadMemory<char>(m_module_base_addr, original_thunk_entry.u1.AddressOfData + 2);
					while (m_parsed_iat.m_entries[i].m_functions[x].m_name[letter_index] && letter_index < 255)
					{
						letter_index++;
						m_parsed_iat.m_entries[i].m_functions[x].m_name[letter_index] = mem_manager.ReadMemory<char>(m_module_base_addr, original_thunk_entry.u1.AddressOfData + 2 + letter_index);
					}
					m_parsed_iat.m_entries[i].m_functions[x].m_name[letter_index] = '\0';
				}

				m_parsed_iat.m_entries[i].m_functions[x].m_address = thunk_entry.u1.AddressOfData;
			}
		}

		/// go through each EAT entry
		const auto eat_entry = mem_manager.ReadMemory<IMAGE_EXPORT_DIRECTORY>(m_module_base_addr, m_parsed_eat.m_base_rva);

		m_parsed_eat.m_num_functions = eat_entry.NumberOfNames;
		m_parsed_eat.m_functions = reinterpret_cast<ParsedEAT::EATFunction*>(malloc(m_parsed_eat.m_num_functions * sizeof(ParsedEAT::EATFunction)));

		for (size_t i = 0; i < m_parsed_eat.m_num_functions; i++)
		{
			const auto address_of_names = mem_manager.ReadMemory<uint32_t>(m_module_base_addr, eat_entry.AddressOfNames + (i * sizeof(uint32_t)));
			const auto address_of_functions = mem_manager.ReadMemory<uint32_t>(m_module_base_addr, eat_entry.AddressOfFunctions + (i * sizeof(uint32_t)));

			/// copy the function name
			{
				size_t letter_index = 0;
				m_parsed_eat.m_functions[i].m_name[letter_index] = mem_manager.ReadMemory<char>(m_module_base_addr + address_of_names);
				while (m_parsed_eat.m_functions[i].m_name[letter_index] && letter_index < 255)
				{
					letter_index++;
					m_parsed_eat.m_functions[i].m_name[letter_index] = mem_manager.ReadMemory<char>(m_module_base_addr + address_of_names + letter_index);
				}
				m_parsed_eat.m_functions[i].m_name[letter_index] = '\0';
			}

			m_parsed_eat.m_functions[i].m_address_address = m_module_base_addr + eat_entry.AddressOfFunctions + (i * sizeof(uint32_t));
			m_parsed_eat.m_functions[i].m_address = m_module_base_addr +
				mem_manager.ReadMemory<uint32_t>(m_parsed_eat.m_functions[i].m_address_address);
		}


		return true;
	}
	void ParsedPEHeader<uint64_t>::Release() const
	{
		/// free memory yeet
		if (m_parsed_iat.m_num_entries && m_parsed_iat.m_entries)
		{
			for (size_t i = 0; i < m_parsed_iat.m_num_entries; i++)
				free(m_parsed_iat.m_entries[i].m_functions);

			free(m_parsed_iat.m_entries);
		}
		if (m_parsed_eat.m_num_functions && m_parsed_eat.m_functions)
			free(m_parsed_eat.m_functions);
	}

	/* x86 */
	bool ParsedPEHeader<uint32_t>::Load(const uint32_t process_id, const char* module_name)
	{
		MemoryManager<uint32_t> mem_manager;
		if (!mem_manager.Initialize(process_id))
			return false;

		m_module_base_addr = mem_manager.GetModuleAddress(module_name);
		if (!m_module_base_addr)
			return false;

		const auto dos_header = mem_manager.ReadMemory<IMAGE_DOS_HEADER>(m_module_base_addr);
		const auto nt_header = mem_manager.ReadMemory<IMAGE_NT_HEADERS>(m_module_base_addr, dos_header.e_lfanew);

		/// make sure the signature of the header is "PE"
		{
			char tmp[4];
			*reinterpret_cast<uint32_t*>(tmp) = nt_header.Signature;
			if (tmp[0] != 'P' || tmp[1] != 'E')
				return false;
		}

		const auto& file_header = nt_header.FileHeader;
		const auto& optional_header = nt_header.OptionalHeader;

		/// get the flippen process type
		m_process_type = PROCESS_TYPE_OTHER;
		if (file_header.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)
			m_process_type = PROCESS_TYPE_EXE;
		else if (file_header.Characteristics & IMAGE_FILE_DLL)
			m_process_type = PROCESS_TYPE_DLL;

		m_num_sections = file_header.NumberOfSections;
		m_time_stamp = file_header.TimeDateStamp;

		/// subsystem
		switch (optional_header.Subsystem)
		{
		case IMAGE_SUBSYSTEM_NATIVE:
			m_process_subsystem = PROCESS_SUBSYSTEM_DRIVER;
			break;
		case IMAGE_SUBSYSTEM_WINDOWS_CUI:
			m_process_subsystem = PROCESS_SUBSYSTEM_CONSOLE;
			break;
		case IMAGE_SUBSYSTEM_WINDOWS_GUI:
			m_process_subsystem = PROCESS_SUBSYSTEM_GUI;
			break;
		default:
			m_process_subsystem = PROCESS_SUBSYSTEM_OTHER;
		}

		/// import and export address table
		m_parsed_eat.m_base_rva = optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		m_parsed_eat.m_size = optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

		m_parsed_iat.m_base_rva = optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		m_parsed_iat.m_size = optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		m_parsed_iat.m_num_entries = (m_parsed_iat.m_size / sizeof(IMAGE_IMPORT_DESCRIPTOR)) - 1;
		m_parsed_iat.m_entries = reinterpret_cast<ParsedIAT::IATEntry*>(malloc(m_parsed_iat.m_num_entries * sizeof(ParsedIAT::IATEntry)));

		/// go through each IAT entry
		for (size_t i = 0; i < m_parsed_iat.m_num_entries; i++)
		{
			const auto iat_entry = mem_manager.ReadMemory<IMAGE_IMPORT_DESCRIPTOR>(m_module_base_addr,
				m_parsed_iat.m_base_rva + uint32_t(i * sizeof(IMAGE_IMPORT_DESCRIPTOR)));

			/// copy the name
			{
				size_t letter_index = 0;
				m_parsed_iat.m_entries[i].m_name[0] = mem_manager.ReadMemory<char>(m_module_base_addr, iat_entry.Name);
				while (m_parsed_iat.m_entries[i].m_name[letter_index] && letter_index < 255)
				{
					letter_index++;
					m_parsed_iat.m_entries[i].m_name[letter_index] = mem_manager.ReadMemory<char>(m_module_base_addr, uint32_t(iat_entry.Name + letter_index));
				}
				m_parsed_iat.m_entries[i].m_name[letter_index] = '\0';
			}

			/// get the number of imported functions the entry contains 
			auto thunk_entry = mem_manager.ReadMemory<IMAGE_THUNK_DATA>(m_module_base_addr, iat_entry.FirstThunk);
			for (m_parsed_iat.m_entries[i].m_num_functions = 0; thunk_entry.u1.AddressOfData != 0;)
			{
				m_parsed_iat.m_entries[i].m_num_functions++;
				thunk_entry = mem_manager.ReadMemory<IMAGE_THUNK_DATA>(m_module_base_addr, iat_entry.FirstThunk + uint32_t(m_parsed_iat.m_entries[i].m_num_functions * sizeof(IMAGE_THUNK_DATA)));
			}

			m_parsed_iat.m_entries[i].m_functions = reinterpret_cast<ParsedIAT::IATEntry::IATFunction*>(malloc(m_parsed_iat.m_entries[i].m_num_functions * sizeof(ParsedIAT::IATEntry::IATFunction)));

			/// iterate through every thunk and gather info :sunglasses:
			for (size_t x = 0; x < m_parsed_iat.m_entries[i].m_num_functions; x++)
			{
				thunk_entry = mem_manager.ReadMemory<IMAGE_THUNK_DATA>(m_module_base_addr, iat_entry.FirstThunk + uint32_t(x * sizeof(IMAGE_THUNK_DATA)));
				m_parsed_iat.m_entries[i].m_functions[x].m_address_address = m_module_base_addr + iat_entry.FirstThunk + uint32_t(x * sizeof(IMAGE_THUNK_DATA));
				const auto original_thunk_entry = mem_manager.ReadMemory<IMAGE_THUNK_DATA>(m_module_base_addr, iat_entry.OriginalFirstThunk + uint32_t(x * sizeof(IMAGE_THUNK_DATA)));

				/// get the name of the function
				{
					size_t letter_index = 0;
					m_parsed_iat.m_entries[i].m_functions[x].m_name[0] = mem_manager.ReadMemory<char>(m_module_base_addr, uint32_t(original_thunk_entry.u1.AddressOfData + 2));
					while (m_parsed_iat.m_entries[i].m_functions[x].m_name[letter_index] && letter_index < 255)
					{
						letter_index++;
						m_parsed_iat.m_entries[i].m_functions[x].m_name[letter_index] = mem_manager.ReadMemory<char>(m_module_base_addr, uint32_t(original_thunk_entry.u1.AddressOfData + 2 + letter_index));
					}
					m_parsed_iat.m_entries[i].m_functions[x].m_name[letter_index] = '\0';
				}

				m_parsed_iat.m_entries[i].m_functions[x].m_address = uint32_t(thunk_entry.u1.AddressOfData);
			}
		}

		/// go through each EAT entry
		const auto eat_entry = mem_manager.ReadMemory<IMAGE_EXPORT_DIRECTORY>(m_module_base_addr, m_parsed_eat.m_base_rva);

		m_parsed_eat.m_num_functions = eat_entry.NumberOfFunctions;
		m_parsed_eat.m_functions = reinterpret_cast<ParsedEAT::EATFunction*>(malloc(m_parsed_eat.m_num_functions * sizeof(ParsedEAT::EATFunction)));

		for (size_t i = 0; i < m_parsed_eat.m_num_functions; i++)
		{
			const auto address_of_names = mem_manager.ReadMemory<uint32_t>(m_module_base_addr, eat_entry.AddressOfNames + uint32_t(i * sizeof(uint32_t)));
			const auto address_of_functions = mem_manager.ReadMemory<uint32_t>(m_module_base_addr, eat_entry.AddressOfFunctions + uint32_t(i * sizeof(uint32_t)));

			/// copy the function name
			{
				size_t letter_index = 0;
				m_parsed_eat.m_functions[i].m_name[letter_index] = mem_manager.ReadMemory<char>(m_module_base_addr + address_of_names);
				while (m_parsed_eat.m_functions[i].m_name[letter_index] && letter_index < 255)
				{
					letter_index++;
					m_parsed_eat.m_functions[i].m_name[letter_index] = mem_manager.ReadMemory<char>(m_module_base_addr + address_of_names + uint32_t(letter_index));
				}
				m_parsed_eat.m_functions[i].m_name[letter_index] = '\0';
			}

			m_parsed_eat.m_functions[i].m_address_address = m_module_base_addr + eat_entry.AddressOfFunctions + uint32_t(i * sizeof(uint32_t));
			m_parsed_eat.m_functions[i].m_address = m_module_base_addr +
				mem_manager.ReadMemory<uint32_t>(m_parsed_eat.m_functions[i].m_address_address);
		}


		return true;
	}
	void ParsedPEHeader<uint32_t>::Release() const
	{
		/// free memory yeet
		if (m_parsed_iat.m_num_entries && m_parsed_iat.m_entries)
		{
			for (size_t i = 0; i < m_parsed_iat.m_num_entries; i++)
				free(m_parsed_iat.m_entries[i].m_functions);

			free(m_parsed_iat.m_entries);
		}
		if (m_parsed_eat.m_num_functions && m_parsed_eat.m_functions)
			free(m_parsed_eat.m_functions);
	}
}