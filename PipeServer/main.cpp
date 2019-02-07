#include <Windows.h>
#include <iostream>

#include "../Bameware Base External/Headers/MemoryManager.h"
#include "../Bameware Base Shared/Headers/FileParser.h"
#include "../Bameware Base Shared/Headers/Pipes/PipeServer.h"

#ifdef _WIN64
#pragma comment(lib, "Bameware Base External_x64.lib")
#pragma comment(lib, "Bameware Base Internal_x64.lib")
#pragma comment(lib, "Bameware Base Shared_x64.lib")
#else
#pragma comment(lib, "Bameware Base External_x86.lib")
#pragma comment(lib, "Bameware Base Internal_x86.lib")
#pragma comment(lib, "Bameware Base Shared_x86.lib")
#endif

#define LOG(x) BAMEWARE::FileParser::WriteToFile("C://Bameware//insurgency_logs.txt", x);


BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID);

void PipeCallback(HANDLE pipe_handle, unsigned char* buffer, size_t bytes_written)
{
	LOG("Pipe: " + std::string((const char*)buffer));
}
void Start()
{
	BAMEWARE::FileParser::CreateNewFile("C://Bameware//insurgency_logs.txt");
	BAMEWARE::FileParser::CreateNewFile("C://Bameware//toggle.txt");

	const auto GetGamePID = []() -> uint32_t
	{
		LOG("Searching for PID...");
		while (true)
		{
			auto processes = BAMEWARE::MemoryManager<uint64_t>::GetProcesses("Steam.exe");

			for (const auto& process : processes)
			{
				for (const auto& mod : process.m_modules)
				{
					LOG("Found PID: " + std::to_string(process.m_process_id));
					return process.m_process_id;
				}
			}

			Sleep(1000);
		}
	};

	/// initialize memory manager
	BAMEWARE::MemoryManager<uint64_t> memory_manager;
	if (!memory_manager.Initialize(GetGamePID()))
	{
		LOG("Failed to initialize memory manager!");
		return;
	}

	LOG("Starting...");

	HANDLE pipe_handle;
	if (!BAMEWARE::PipeServer::Get()->CreatePipeServer(PipeCallback, "BootyCheeks", &pipe_handle))
	{
		LOG("Failed to create pipe server!");
		std::cout << "NUTS" << std::endl;
		return;
	}
	LOG("Created pipe server!");
	while (BAMEWARE::PipeServer::Get()->UpdatePipeServers())
	{
		if (!BAMEWARE::FileParser::DoesFileExist("C://Bameware//toggle.txt"))
			BAMEWARE::PipeServer::Get()->DestroyPipeServer(pipe_handle);
	}

	LOG("Ending...");
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(Start), nullptr, NULL, nullptr);
		break;
	}
	}

	return TRUE;
}