#pragma once

#include "CompileTimeRandom.h"

#include <array>


#pragma warning( disable : 4307)


#define ENCRYPT_STRING(x) BAMEWARE::CRYPTOGRAPHY::EncryptedString<sizeof(x)>(x, std::make_index_sequence<sizeof(x)>()).DecryptString()


namespace BAMEWARE::CRYPTOGRAPHY
{
	template <size_t string_len, const char random_seed = CompileTimeRandom<char, string_len, string_len * 5>().value>
	class EncryptedString
	{
	public:
		template <size_t... index>
		constexpr EncryptedString(const char* str, std::index_sequence< index... >) : m_array{ EncryptChar<index>(str[index])... } {}

		char* DecryptString()
		{
			for (size_t i = 0; i < string_len; i++)
				m_array[i] = DecryptChar(m_array[i], i);

			return m_array.data();
		}

	private:
		std::array<char, string_len> m_array;

	private:
		template <size_t i>
		static constexpr char EncryptChar(char c)
		{
			return char((c + random_seed) ^ (static_cast<unsigned char>(random_seed) * (i + 1)));
		}

		static char DecryptChar(char c, size_t i)
		{
			return char((c ^ (static_cast<unsigned char>(random_seed) * (i + 1))) - random_seed);
		}
	};
}