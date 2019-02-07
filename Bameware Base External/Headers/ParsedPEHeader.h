#pragma once

#include <cstdint>


namespace BAMEWARE
{
	template <typename ptr>
	class ParsedPEHeader
	{
	public:
		bool Load(uint32_t process_id, const char* module_name); /// no need to call Release() if this returns false
		void Release() const;

	public:
		enum PROCESS_TYPE
		{
			PROCESS_TYPE_OTHER = 0,
			PROCESS_TYPE_DLL,
			PROCESS_TYPE_EXE
		};
		enum PROCESS_SUBSYSTEM
		{
			PROCESS_SUBSYSTEM_OTHER = 0,
			PROCESS_SUBSYSTEM_DRIVER,
			PROCESS_SUBSYSTEM_GUI,
			PROCESS_SUBSYSTEM_CONSOLE
		};

	public:
		struct ParsedEAT
		{
			struct EATFunction
			{
				char m_name[256];
				ptr m_address,
					m_address_address;
			};

			ptr m_base_rva,
				m_size;
			size_t m_num_functions;
			EATFunction* m_functions;
		};
		struct ParsedIAT
		{
			/// theres usually an IATEntry for every module imported from
			struct IATEntry
			{
				struct IATFunction
				{
					char m_name[256]; /// this would be the function name
					ptr m_address,
						m_address_address;
				};

				char m_name[256]; /// this would be the module name
				size_t m_num_functions;
				IATFunction* m_functions;
			};

			ptr m_base_rva,
				m_size;
			size_t m_num_entries;
			IATEntry* m_entries;
		};

	public:
		ptr m_module_base_addr;

		PROCESS_TYPE m_process_type;
		PROCESS_SUBSYSTEM m_process_subsystem;

		size_t m_num_sections;
		uint32_t m_time_stamp; /// number of seconds since December 31st, 1969, at 4:00 P.M

		ParsedEAT m_parsed_eat;
		ParsedIAT m_parsed_iat;
	};
}