#pragma once

#include <Windows.h>


namespace BAMEWARE
{
	class PipeClient
	{
	public:
		PipeClient() : m_pipe_handle(INVALID_HANDLE_VALUE) {}
		explicit PipeClient(const char* pipe_server_name) { Initialize(pipe_server_name); }
		~PipeClient() { Release(); }

	public:
		bool Initialize(const char* pipe_server_name);
		void Release();

		bool WriteToServer(const void* buffer, size_t num_bytes) const;
		bool WriteToServer(const char* buffer, const size_t num_bytes) const { return WriteToServer(reinterpret_cast<const void*>(buffer), num_bytes); }

		/// returns how many bytes read
		size_t ReadFromServer(void* buffer, size_t buffer_size) const;
		size_t ReadFromServer(char* buffer, const size_t buffer_size) const { return ReadFromServer(reinterpret_cast<void*>(buffer), buffer_size); }

	private:
		HANDLE m_pipe_handle = INVALID_HANDLE_VALUE;

	private:
	};	
}