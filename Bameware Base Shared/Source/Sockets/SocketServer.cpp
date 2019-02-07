#include "../../Headers/Sockets/SocketServer.h"

#include <iostream>


namespace BAMEWARE
{
	void ListenerThread(SocketServer* server, const SOCKET socket)
	{
		server->ListenerCallback(socket);
	}

	void AcceptClientsThread(SocketServer* server)
	{
		server->AcceptClientsCallback();
	}

	void SocketServer::InitializeSockets()
	{
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(2, 0), &wsa_data);
	}

	void SocketServer::ReleaseSockets()
	{
		WSACleanup();
	}

	void SocketServer::OpenServer(const unsigned short port, const std::string& password)
	{
		m_password = password;
		m_server_socket = socket(AF_INET, SOCK_STREAM, 0);

		SOCKADDR_IN server_address;
		server_address.sin_addr.s_addr = INADDR_ANY;
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(port);

		bind(m_server_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address));
		listen(m_server_socket, 0);

		m_accept_clients_thread = std::thread(AcceptClientsThread, this);
	}

	void SocketServer::CloseServer()
	{
		m_accept_clients_thread.join();

		for (auto& client : m_clients)
		{
			if (client.second.m_is_connected)
				client.second.m_listener_thread.join();
		}

		closesocket(m_server_socket);
	}

	void SocketServer::SendMessageToClient(const SOCKET client, const std::string& message)
	{
		send(client, message.c_str(), message.size() + 1, 0);
	}

	void SocketServer::ListenerCallback(const SOCKET socket)
	{
		while (socket != INVALID_SOCKET)
		{
			char buffer[1024];
			const int ret = recv(socket, buffer, 1024, 0);

			/// client disconnected
			if (ret <= 0)
			{
				m_clients.at(socket).m_is_connected = false;
				closesocket(socket);

				if (m_disconnect_callback)
					m_disconnect_callback(this, m_clients.at(socket));

				break;
			}

			m_clients.at(socket).m_recieved_messages.emplace_back(buffer);

			if (m_message_callback)
				m_message_callback(this, m_clients.at(socket), buffer);
		}
	}

	void SocketServer::AcceptClientsCallback()
	{
		int client_address_size = sizeof(SOCKADDR_IN);
		SOCKADDR_IN client_address;
		while (auto client_socket = accept(m_server_socket, reinterpret_cast<sockaddr*>(&client_address),
		                                   &client_address_size))
		{
			std::cout << "Client attempting to connect: " << client_socket << std::endl;

			char buffer[1024];
			if (recv(client_socket, buffer, 1024, 0) <= 0)
				continue;

			std::string password = buffer;
			if (password.size() < 9)
				continue;

			password = password.substr(9, password.size() - 9);
			if (password != m_password)
			{
				std::cout << "Client rejected due to incorrect password!" << std::endl;
				send(client_socket, "PASSWORD_FAIL", 14, 0);
				closesocket(client_socket);
				continue;
			}

			send(client_socket, "PASSWORD_SUCCESS", 17, 0);

			m_clients[client_socket] = ClientInfo();

			m_clients.at(client_socket).m_client_id = m_clients.size();
			m_clients.at(client_socket).m_is_connected = true;
			m_clients.at(client_socket).m_socket = client_socket;
			m_clients.at(client_socket).m_listener_thread = std::thread(ListenerThread, this, client_socket);

			if (m_connect_callback)
				m_connect_callback(this, m_clients.at(client_socket));
		}
	}
}
