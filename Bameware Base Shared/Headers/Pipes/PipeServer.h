#pragma once

#include <Windows.h>
#include <unordered_map>


namespace BAMEWARE
{
	class PipeServer
	{
	public:
		static PipeServer* Get()
		{
			static PipeServer instance;
			return &instance;
		}

	public:
		typedef void(*PipeServerCallbackFn)(HANDLE pipe_handle, unsigned char* buffer, size_t bytes_written);
		static constexpr size_t pipe_server_buffer_size = 1024;

	public:
		PipeServerCallbackFn GetPipeServerCallback(HANDLE pipe_handle) const;

		bool CreatePipeServer(PipeServerCallbackFn callback, const char* name, HANDLE* out_pipe_handle = nullptr);
		void DestroyPipeServer(HANDLE pipe_handle);

		bool UpdatePipeServers() const { return !m_pipe_servers.empty(); }

		static bool WriteToClient(HANDLE pipe_handle, const void* buffer, size_t num_bytes);
		bool WriteToClient(const HANDLE pipe_handle, const char* buffer, const size_t num_bytes) const { return WriteToClient(pipe_handle, reinterpret_cast<const void*>(buffer), num_bytes); }

	private:
		std::unordered_map<HANDLE, PipeServerCallbackFn> m_pipe_servers;

	public:
		PipeServer() = default;
		PipeServer(const PipeServer&) = delete;
		void operator=(const PipeServer&) = delete;
	};	
}