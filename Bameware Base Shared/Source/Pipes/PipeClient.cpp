#include "../../Headers/Pipes/PipeClient.h"
#include "../../Headers/Pipes/PipeServer.h"

#include <iostream>
#include "../../Headers/Utils.h"


namespace BAMEWARE
{
	bool PipeClient::Initialize(const char* pipe_server_name)
	{
		if (m_pipe_handle && m_pipe_handle != INVALID_HANDLE_VALUE)
			return false;

		char pipe_name[256];
		sprintf_s(pipe_name, R"(\\.\pipe\%s)", pipe_server_name);

		m_pipe_handle = CreateFile(pipe_name, GENERIC_READ | GENERIC_WRITE,
		                           0, nullptr, OPEN_EXISTING, 0, nullptr);

		if (m_pipe_handle == INVALID_HANDLE_VALUE)
		{
			//std::cout << UTILS::GetLastErrorAsString() << std::endl;
			return false;
		}

		return true;
	}

	void PipeClient::Release()
	{
		if (m_pipe_handle && m_pipe_handle != INVALID_HANDLE_VALUE)
			CloseHandle(m_pipe_handle);

		m_pipe_handle = nullptr;
	}

	bool PipeClient::WriteToServer(const void* buffer, const size_t num_bytes) const
	{
		if (!m_pipe_handle || m_pipe_handle == INVALID_HANDLE_VALUE)
			return false;

		if (num_bytes >= PipeServer::pipe_server_buffer_size)
			return false;

		unsigned long bytes_written;
		WriteFile(m_pipe_handle, buffer, static_cast<unsigned long>(num_bytes), &bytes_written, nullptr);

		return true;
	}

	size_t PipeClient::ReadFromServer(void* buffer, const size_t buffer_size) const
	{
		unsigned long bytes_read;
		if (!ReadFile(m_pipe_handle, buffer, static_cast<unsigned long>(buffer_size), &bytes_read, nullptr))
			return 0;

		return bytes_read;
	}
}
