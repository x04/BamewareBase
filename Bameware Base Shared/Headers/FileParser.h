#pragma once

#include <string>
#include <vector>


namespace BAMEWARE
{
	class FileParser
	{
	public:
		bool LoadFile(const std::string& file_path);
		bool SaveFile(const std::string& file_path) const;
		void Clear();

	public:
		struct TextData
		{
			std::string m_name,
				m_comment;
			std::vector<std::string> m_arguments;
		};
		struct TextSection
		{
			std::string m_name,
				m_comment;
			std::vector<TextData> m_data;
			std::vector<TextSection> m_sections;
		};

	public:
		TextSection* CreateNewTextSection(const std::string& section_name, TextSection* parent_section = nullptr);
		static TextData* CreateNewTextData(const std::string& data_name, TextSection& section, size_t num_args, ...);

		TextSection* FindTextSection(const std::string& section_name, TextSection* parent_section = nullptr);
		static TextData* FindTextData(const std::string& data_name, TextSection& section);

		/// creates a new section/data if none exists
		TextSection* SafeFindTextSection(const std::string& section_name, TextSection* parent_section = nullptr);
		static TextData* SafeFindTextData(const std::string& data_name, TextSection& section, size_t num_args, ...);

		std::vector<TextSection>& GetTextSections() { return m_text_sections; }

	public:
		static bool DoesFileExist(const std::string& file_path);
		static void CreateNewFile(const std::string& file_path); /// can also be used to empty an existing file
		static void WriteToFile(const std::string& file_path, const std::string& message);

	private:
		std::vector<TextSection> m_text_sections;
	};
}
