#include "../../Headers/Sockets/SocketClient.h"
#include <iostream>


namespace BAMEWARE
{
	void ListeningThread(SocketClient* client)
	{
		client->SetIsListening(true);

		while (client->GetSocket() != INVALID_SOCKET && !client->ShouldStopListening())
		{
			char buffer[1024];
			const int ret = recv(client->GetSocket(), buffer, 1024, 0);

			if (ret <= 0)
			{
				client->SetIsListening(false);
				client->DisconnectFromServer();
				break;
			}
			client->PushRecievedMessage(buffer);
		}

		client->SetIsListening(false);
	}


	SocketClient::SocketClient(const std::string& ipv4, const unsigned short port, const std::string& password)
	{
		if (!ConnectToServer(ipv4, port, password))
			throw std::runtime_error("Failed to connect to server!");
	}

	void SocketClient::InitializeSockets()
	{
		WSADATA wsa_data;
		WSAStartup(MAKEWORD(2, 0), &wsa_data);
	}

	void SocketClient::ReleaseSockets()
	{
		WSACleanup();
	}

	bool SocketClient::ConnectToServer(const std::string& ipv4, const unsigned short port, const std::string& password)
	{
		m_socket = socket(AF_INET, SOCK_STREAM, 0);

		SOCKADDR_IN address;
		address.sin_addr.s_addr = inet_addr(ipv4.c_str());
		address.sin_family = AF_INET;
		address.sin_port = htons(port);

		connect(m_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
		SendMessageToServer(std::string("PASSWORD:") + password);

		char buffer[1024];
		if (recv(m_socket, buffer, 1024, 0) <= 0)
			return false;

		if (!strcmp(buffer, "PASSWORD_FAIL"))
			return false;

		return m_socket != INVALID_SOCKET;
	}

	void SocketClient::DisconnectFromServer()
	{
		if (m_socket != INVALID_SOCKET)
			closesocket(m_socket);

		m_socket = INVALID_SOCKET;
	}

	void SocketClient::StartListening()
	{
		m_listening_thread = std::thread(ListeningThread, this);
	}

	void SocketClient::StopListening()
	{
		m_should_stop_listening = true;
		std::cout << "Waiting for thread to finish..." << std::endl;
		m_listening_thread.join();
		std::cout << "Thread finished." << std::endl;
		m_should_stop_listening = false;
	}

	void SocketClient::SendMessageToServer(const std::string& message) const
	{
		send(m_socket, message.c_str(), message.size() + 1, 0);
	}
}
