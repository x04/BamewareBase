#include "../../Headers/Pipes/PipeServer.h"

#include "../../Headers/Utils.h"

#include "../../Headers/FileParser.h"


#define LOG(x) BAMEWARE::FileParser::WriteToFile("C://Bameware//pipe_logs.txt", x);


namespace BAMEWARE
{
	void PipeServerThread(LPVOID parameter)
	{
		const auto pipe_callback = PipeServer::Get()->GetPipeServerCallback(parameter);
		if (!pipe_callback)
			return;

		unsigned char buffer[PipeServer::pipe_server_buffer_size];
		while (ConnectNamedPipe(parameter, nullptr))
		{
			unsigned long num_bytes_read;
			while (ReadFile(parameter, buffer, PipeServer::pipe_server_buffer_size - 1, &num_bytes_read, nullptr))
				pipe_callback(parameter, buffer, num_bytes_read);

			DisconnectNamedPipe(parameter);
		}

		PipeServer::Get()->DestroyPipeServer(parameter);
	}

	bool PipeServer::CreatePipeServer(const PipeServerCallbackFn callback, const char* name, HANDLE* out_pipe_handle)
	{
		char pipe_name[256];
		sprintf_s(pipe_name, R"(\\.\pipe\%s)", name);

		const auto pipe_handle = CreateNamedPipe(pipe_name, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE |
		                                         PIPE_WAIT, 1, pipe_server_buffer_size, pipe_server_buffer_size,
		                                         NMPWAIT_USE_DEFAULT_WAIT, nullptr);

		if (out_pipe_handle)
			*out_pipe_handle = pipe_handle;

		if (pipe_handle == INVALID_HANDLE_VALUE)
		{
			//LOG(std::string("MANGO: ") + UTILS::GetLastErrorAsString());
			return false;
		}

		m_pipe_servers[pipe_handle] = callback;
		CreateThread(nullptr, NULL, LPTHREAD_START_ROUTINE(PipeServerThread), pipe_handle, NULL, nullptr);

		return true;
	}

	void PipeServer::DestroyPipeServer(HANDLE pipe_handle)
	{
		if (m_pipe_servers.find(pipe_handle) == m_pipe_servers.end())
			return;

		m_pipe_servers.erase(pipe_handle);
		CloseHandle(pipe_handle);
	}

	bool PipeServer::WriteToClient(HANDLE pipe_handle, const void* buffer, const size_t num_bytes)
	{
		if (!pipe_handle || pipe_handle == INVALID_HANDLE_VALUE)
			return false;

		if (num_bytes >= pipe_server_buffer_size)
			return false;

		unsigned long bytes_written;
		WriteFile(pipe_handle, buffer, static_cast<unsigned long>(num_bytes), &bytes_written, nullptr);

		return true;
	}

	PipeServer::PipeServerCallbackFn PipeServer::GetPipeServerCallback(HANDLE pipe_handle) const
	{
		if (m_pipe_servers.find(pipe_handle) == m_pipe_servers.end())
			return nullptr;

		return m_pipe_servers.at(pipe_handle);
	}
}
