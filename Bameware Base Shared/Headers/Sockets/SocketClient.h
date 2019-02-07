#pragma once

#include <string>
#include <thread>
#include <vector>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")


namespace BAMEWARE
{
	class SocketClient
	{
	public:
		SocketClient() = default;
		SocketClient(const SocketClient&) = delete;
		SocketClient(const std::string& ipv4, unsigned short port, const std::string& password = "");

	public:
		/// call both of these only once, no matter the number of socket clients
		static void InitializeSockets();
		static void ReleaseSockets();

	public:
		bool ConnectToServer(const std::string& ipv4, unsigned short port, const std::string& password = "");
		void DisconnectFromServer();

		void SendMessageToServer(const std::string& message) const;

		/// start and stop a 
		void StartListening();
		void StopListening();

	public:
		bool IsListening() const { return m_is_listening; }
		void SetIsListening(const bool value) { m_is_listening = value; }
		bool ShouldStopListening() const { return m_should_stop_listening; }

		SOCKET& GetSocket() { return m_socket; }

		std::vector<std::string>& GetRecievedMessages() { return m_recieved_messages; }
		void PushRecievedMessage(const std::string& message) { m_recieved_messages.push_back(message); }

	private:
		SOCKET m_socket = INVALID_SOCKET;
		std::thread m_listening_thread;
		std::vector<std::string> m_recieved_messages;
		bool m_is_listening = false,
		     m_should_stop_listening = false;
	};
}
