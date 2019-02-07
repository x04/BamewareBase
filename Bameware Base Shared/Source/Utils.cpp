#include "../Headers/Utils.h"

#include <Windows.h>
#include <Iptypes.h>
#include <iphlpapi.h>
#include <Shlwapi.h>
#include <random>
#include <tchar.h>
#include <strsafe.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include "../Headers/Cryptography/StringEncryption.h"


namespace BAMEWARE::UTILS
{
	void AllocateConsole(const char* console_title)
	{
		AllocConsole();

		SetConsoleTitle(console_title);
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
	}

	float RandomNumber(const float min, const float max)
	{
		static std::random_device rand_device;
		static auto rand_gen = std::mt19937(rand_device());

		const std::uniform_real_distribution<float> rand_distributer(min, max);
		return rand_distributer(rand_gen);
	}

	std::string GetLastErrorAsString()
	{
		const auto last_error = GetLastError();
		if (!last_error)
			return std::string("");

		char* message_buffer = nullptr;
		const size_t size = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<char*>(&message_buffer), 0,
			nullptr);

		std::string message(message_buffer, size);
		LocalFree(message_buffer);

		return message;
	}
}
