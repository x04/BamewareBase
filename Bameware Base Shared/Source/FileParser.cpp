#include "../Headers/FileParser.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <stack>
#include <cstdarg>


namespace BAMEWARE
{
	void RecursivelySaveSection(const FileParser::TextSection& section, std::fstream& stream, const size_t indent = 0)
	{
		stream << std::string(indent, ' ') << "\"" << section.m_name << "\" {";
		if (!section.m_comment.empty())
			stream << R"( //)" << section.m_comment;
		stream << std::endl;

		for (const auto& data : section.m_data)
		{
			stream << std::string(indent + 4, ' ') << "\"" << data.m_name << "\"";
			for (const auto& arg : data.m_arguments)
				stream << " \"" << arg << "\"";

			if (!data.m_comment.empty())
				stream << R"( //)" << data.m_comment;

			stream << std::endl;
		}

		for (const auto& child_section : section.m_sections)
			RecursivelySaveSection(child_section, stream, indent + 4);

		stream << std::string(indent, ' ') << "}" << std::endl;
	}

	bool FileParser::LoadFile(const std::string& file_path)
	{
		std::vector<std::string> file_contents;
		{
			std::fstream file(file_path, std::ios::out | std::ios::in);
			if (!file.is_open())
				return false;

			std::string line;
			while (std::getline(file, line))
				file_contents.emplace_back(line);

			file.close();
		}

		/// returns the comment and removes the comment from the line
		const auto RemoveComment = [](std::string& line) -> std::string
		{
			std::string comment;

			if (line.empty())
				return comment;

			for (size_t i = line.size() - 1; i >= 1; i--)
			{
				if (line[i] == '/' && line[i - 1] == '/')
				{
					comment = line.substr(i + 1, (line.size() - (i - 3)));
					line.erase(line.begin() + (i - 1), line.end());
				}
			}

			return comment;
		};
		const auto GetArgs = [](const std::string& line) -> std::vector<std::string>
		{
			std::vector<std::string> args;

			bool is_in_quotation = false;
			std::string cached_string;
			for (char i : line)
			{
				if (i == '\"')
				{
					is_in_quotation = !is_in_quotation;

					if (!is_in_quotation)
					{
						args.push_back(cached_string);
						cached_string.clear();
					}
				}
				else
				{
					if (is_in_quotation)
						cached_string += i;
				}
			}

			return args;
		};

		/// FindFirstChar and FindLastChar return ' ' if no success
		const auto FindFirstChar = [](const std::string& line) -> char
		{
			if (line.empty())
				return ' ';

			for (char i : line)
			{
				if (i == ' ')
					continue;

				return i;
			}

			return ' ';
		};
		const auto FindLastChar = [](const std::string& line) -> char
		{
			if (line.empty())
				return ' ';

			for (size_t i = line.size() - 1; i >= 0; i--)
			{
				if (line[i] == ' ')
					continue;

				return line[i];
			}

			return ' ';
		};

		std::stack<TextSection*> section_stack;
		for (auto& line : file_contents)
		{
			const auto comment = RemoveComment(line);

			const auto args = GetArgs(line);

			/// start of a new section
			if (args.size() == 1 && FindLastChar(line) == '{')
			{
				TextSection section;

				section.m_name = args.front();
				section.m_comment = comment;

				if (section_stack.empty())
				{
					m_text_sections.push_back(section);
					section_stack.push(&m_text_sections.back());
				}
				else
				{
					section_stack.top()->m_sections.push_back(section);
					section_stack.push(&section_stack.top()->m_sections.back());
				}
			}
			else if (!section_stack.empty())
			{
				/// end of a section
				if (FindFirstChar(line) == '}')
					section_stack.pop();
				else if (!args.empty()) /// data
				{
					TextData data;

					data.m_name = args.front();
					data.m_comment = comment;

					for (size_t i = 1; i < args.size(); i++)
						data.m_arguments.push_back(args[i]);

					section_stack.top()->m_data.push_back(data);
				}
			}
		}

		return true;
	}

	bool FileParser::SaveFile(const std::string& file_path) const
	{
		std::fstream file(file_path, std::ios::out | std::ios::in | std::ios::trunc);
		if (!file.is_open())
			return false;

		for (const auto& section : m_text_sections)
			RecursivelySaveSection(section, file);

		file.close();
		return true;
	}

	void FileParser::Clear()
	{
		m_text_sections.clear();
	}

	FileParser::TextSection* FileParser::CreateNewTextSection(const std::string& section_name,
	                                                          TextSection* parent_section)
	{
		if (!parent_section)
		{
			TextSection section;
			section.m_name = section_name;
			m_text_sections.push_back(section);
			return &m_text_sections.back();
		}
		TextSection section;
		section.m_name = section_name;
		parent_section->m_sections.push_back(section);
		return &parent_section->m_sections.back();
	}

	FileParser::TextData* FileParser::CreateNewTextData(const std::string& data_name, TextSection& section,
	                                                    size_t num_args, ...)
	{
		va_list arguments;
		va_start(arguments, num_args);

		TextData data;
		data.m_name = data_name;

		for (size_t i = 0; i < num_args; i++)
			data.m_arguments.emplace_back(va_arg(arguments, const char*));

		va_end(arguments);

		section.m_data.push_back(data);
		return &section.m_data.back();
	}

	FileParser::TextSection* FileParser::FindTextSection(const std::string& section_name, TextSection* parent_section)
	{
		if (!parent_section)
		{
			for (auto& section : m_text_sections)
			{
				if (section.m_name == section_name)
					return &section;
			}
		}
		else
		{
			for (auto& section : parent_section->m_sections)
			{
				if (section.m_name == section_name)
					return &section;
			}
		}

		return nullptr;
	}

	FileParser::TextData* FileParser::FindTextData(const std::string& data_name, TextSection& section)
	{
		for (auto& data : section.m_data)
		{
			if (data.m_name == data_name)
				return &data;
		}

		return nullptr;
	}

	FileParser::TextSection* FileParser::SafeFindTextSection(const std::string& section_name,
	                                                         TextSection* parent_section)
	{
		/// a section already exists, just return that one
		if (const auto section = FindTextSection(section_name, parent_section); section)
			return section;

		/// create a new section
		if (!parent_section)
		{
			TextSection section;
			section.m_name = section_name;
			m_text_sections.push_back(section);
			return &m_text_sections.back();
		}
		TextSection section;
		section.m_name = section_name;
		parent_section->m_sections.push_back(section);
		return &parent_section->m_sections.back();
	}

	FileParser::TextData* FileParser::SafeFindTextData(const std::string& data_name, TextSection& section,
	                                                   size_t num_args, ...)
	{
		/// data already exists, just return that one
		if (const auto data = FindTextData(data_name, section); data)
			return data;

		/// create a new data object
		va_list arguments;
		va_start(arguments, num_args);

		TextData data;
		data.m_name = data_name;

		for (size_t i = 0; i < num_args; i++)
			data.m_arguments.emplace_back(va_arg(arguments, const char*));

		va_end(arguments);

		section.m_data.push_back(data);
		return &section.m_data.back();
	}

	bool FileParser::DoesFileExist(const std::string& file_path)
	{
		return std::experimental::filesystem::exists(file_path);
	}

	void FileParser::CreateNewFile(const std::string& file_path)
	{
		std::fstream file(file_path, std::ios::trunc | std::ios::out);
		file.close();
	}

	void FileParser::WriteToFile(const std::string& file_path, const std::string& message)
	{
		if (!DoesFileExist(file_path))
			return;

		std::fstream file(file_path, std::ios::out | std::ios::app);
		if (!file.is_open())
			return;

		file << message << std::endl;
		file.close();
	}
}
