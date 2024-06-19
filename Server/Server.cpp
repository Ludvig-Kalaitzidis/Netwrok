#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <vector>
#include "NetMessage.h"
#include <unordered_map>
#include <tge\math\Vector.h>
#pragma comment (lib, "ws2_32.lib")

using namespace std;
#define BUFFER_SIZE 512
#define MESSAGE_SIZE 300

#define PORT 54000


struct ClientInfo
{
	sockaddr_in address;
	std::string name;
	int clientId;
	
};
void DeleteTheClient(const SOCKET& aServerSocket, const int aClientID);
void AddNewClient(const SOCKET& aServerSocket, const sockaddr_in& aClientAddress, ChatMessage* aClientMessage);
//client port till clientInfo
std::unordered_map<int, ClientInfo> connectedClients;

int main()
{
	// Startup winsock
	WSADATA data; //winsock data
	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		cout << "Can't start winsock! " << wsOk << endl;
		return 2;
	}

	//Creat server socket
	SOCKET listeningSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (listeningSocket == INVALID_SOCKET) {
		std::cerr << "Failed to create listening socket." << std::endl;
		return 1;
	}
	//creat two socket for server for the threading (send and recive)

	// socket information
	sockaddr_in serverAddres;
	serverAddres.sin_addr.S_un.S_addr = ADDR_ANY;
	serverAddres.sin_family = AF_INET;
	serverAddres.sin_port = htons(PORT); // convert from little to big endian

	if (bind(listeningSocket, (sockaddr*)&serverAddres, sizeof(serverAddres)) == SOCKET_ERROR)
	{
		cout << "Can't bind socket! " << WSAGetLastError() << endl;
		return 2;
	}
	std::cout << "Server is online on port " << PORT << std::endl;


	while (true)
	{
		sockaddr_in clientAddress;
		int clientAddressSize = sizeof(clientAddress);

		char buffer[BUFFER_SIZE];
		int bytesReceived = recvfrom(listeningSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddress, &clientAddressSize);
		if (bytesReceived == SOCKET_ERROR)
		{
			std::cerr << "Error receiving data from client." << std::endl;
			continue;
		}

		//When client is new
		ChatMessage* clientMessage = reinterpret_cast<ChatMessage*>(&buffer);
		if (clientMessage->GetClientID() == -1)
		{
			AddNewClient(listeningSocket, clientAddress, clientMessage);
			continue;
		}

		//When a client disconnect 
		if ((MessageState)buffer[1] == MessageState::LastSend)
		{
			DeleteTheClient(listeningSocket, clientMessage->GetClientID());
			continue;
		}
		

		//Echo the message back
		
		sendto(listeningSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddress, sizeof(clientAddress));

		//Display (name + client) message and send them to all connected clients
	
		std::string wholeMessage = connectedClients[clientMessage->GetClientID()].name + ": " + clientMessage->GetClientMessage();
		clientMessage->SetMessage(wholeMessage.c_str());
		for (const auto& client : connectedClients)
		{
			if (client.second.clientId != clientMessage->GetClientID())
			{
				sendto(listeningSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&client.second.address, sizeof(client.second.address));
			}
		}

		
		if ((MessageType)buffer[0] == MessageType::ePositsionMessage)
		{

		}
		else
		{
		std::cout << clientMessage->GetClientMessage() << std::endl;

		}//display the message on server
	}

	// close socket 
	closesocket(listeningSocket);

	// shoutdown winsock
	WSACleanup();

	system("pause");
}


void DeleteTheClient(const SOCKET& aServerSocket, const int aClientID)
{
	//Display on server
	std::cout << connectedClients[aClientID].name << " : Has Disconnected!" << endl;

	//send the disconnect message to all connected clients
	ChatMessage disconnectMessage;
	std::string message = connectedClients[aClientID].name + " : Has Disconnected!";
	char disconnectBuffer[BUFFER_SIZE];
	disconnectMessage.SetMessage(message.c_str());
	disconnectMessage.SetState(MessageState::None);
	memcpy(disconnectBuffer, &disconnectMessage, BUFFER_SIZE);

	for (const auto& client : connectedClients)
	{
		if (client.second.clientId != aClientID)
		{
			sendto(aServerSocket, disconnectBuffer, BUFFER_SIZE, 0, (sockaddr*)&client.second.address, sizeof(client.second.address));
		}
	}

	//delte the clients
	connectedClients.erase(aClientID);
}

void AddNewClient(const SOCKET& aServerSocket, const sockaddr_in& aClientAddress, ChatMessage* aClientMessage)
{
	ClientInfo newClientInfo;
	newClientInfo.address = aClientAddress;
	static unsigned int clientID = 0;
	newClientInfo.clientId = clientID;
	newClientInfo.name = aClientMessage->GetClientMessage();
	std::cout << aClientMessage->GetClientMessage() << " is Online.. ID: " << clientID << std::endl;

	connectedClients[clientID] = newClientInfo;
	std::cout << "Clients size now is: " << connectedClients.size() << std::endl;

	//send to client to confirm connection
	//ZeroMemory(buffer, BUFFER_SIZE);
	char NewClientBuffer[BUFFER_SIZE];

	ChatMessage connectionInfo;
	std::string messageToAllClients = newClientInfo.name + " is Online";
	connectionInfo.SetMessage(messageToAllClients.c_str());
	connectionInfo.SetID(clientID);
	connectionInfo.SetState(MessageState::FirstSend);
	memcpy(NewClientBuffer, &connectionInfo, BUFFER_SIZE);

	for (const auto& entry : connectedClients)
	{
		sendto(aServerSocket, NewClientBuffer, BUFFER_SIZE, 0, (sockaddr*)&entry.second.address, sizeof(entry.second.address));
	}

	//char NewNewClientBuffer[BUFFER_SIZE];
	//FirstMessage connectedClientsMessage;
	//connectedClientsMessage.SetConnectedClients(connectedClients);
	//sendto(aServerSocket, NewNewClientBuffer, BUFFER_SIZE, 0, (sockaddr*)&newClientInfo.address, sizeof(newClientInfo.address));

	clientID++;
}
