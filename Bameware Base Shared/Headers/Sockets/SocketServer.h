#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <thread>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")


namespace BAMEWARE
{
	class SocketServer
	{
	public:
		/// call both of these only once, no matter the number of socket servers
		static void InitializeSockets();
		static void ReleaseSockets();

	public:
		struct ClientInfo
		{
			bool m_is_connected = false;
			unsigned int m_client_id = -1;

			SOCKET m_socket{};
			std::thread m_listener_thread;
			std::vector<std::string> m_recieved_messages;
		};

		typedef void (*ClientMessageCallbackFn)(SocketServer* server, ClientInfo& client, const std::string& message);
		typedef void (*ClientConnectCallbackFn)(SocketServer* server, ClientInfo& client);
		typedef void (*ClientDisconnectCallbackFn)(SocketServer* server, ClientInfo& client);

	public:
		void OpenServer(unsigned short port, const std::string& password = "");
		void CloseServer();
		/// doesn't actually close the server, just waits for all threads to complete (will never happen)

		static void SendMessageToClient(SOCKET client, const std::string& message);

		void RegisterClientMessageCallback(const ClientMessageCallbackFn callback) { m_message_callback = callback; }
		void RegisterClientConnectCallback(const ClientConnectCallbackFn callback) { m_connect_callback = callback; }

		void RegisterClientDisconnectCallback(const ClientDisconnectCallbackFn callback)
		{
			m_disconnect_callback = callback;
		}

		std::unordered_map<SOCKET, ClientInfo>& GetClients() { return m_clients; }

		void ListenerCallback(SOCKET socket);
		void AcceptClientsCallback();

	private:
		SOCKET m_server_socket{};
		std::string m_password;
		std::thread m_accept_clients_thread;
		std::unordered_map<SOCKET, ClientInfo> m_clients;

		ClientMessageCallbackFn m_message_callback = nullptr;
		ClientConnectCallbackFn m_connect_callback = nullptr;
		ClientDisconnectCallbackFn m_disconnect_callback = nullptr;
	};
}
