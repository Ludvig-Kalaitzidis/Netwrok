#include <iostream>
#include <WS2tcpip.h>
#include <limits>
#pragma comment (lib, "ws2_32.lib")
#include "NetMessage.h"
using namespace std;
#define BUFFER_SIZE 512
#define MESSAGE_SIZE 300
#define PORT 54000
#define IP_ADDRESS "127.0.0.1"
#ifdef max
#undef max
#endif



#include <thread>

bool SendToServer(const SOCKET& aServerScoket, const sockaddr_in& aServerAddress);
bool ResiveFromServer(const SOCKET& aServerScoket, const sockaddr_in& aServerAddress);

int globalID = -1;

BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType);

int main()
{

	// Startup winsock
	WSADATA data; //winsock data
	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		cout << "Kan inte starta vinststrumpa! " << wsOk << endl;
		return 2;
	}

	//Socket creation
	SOCKET ClientSocket = socket(AF_INET, SOCK_DGRAM, 0);


	//creat two socket for server for the threading (send and recive)

	//a hint structure for the server
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(54000); // convert from little to big endian
	inet_pton(AF_INET, IP_ADDRESS, &serverAddress.sin_addr);

	ChatMessage message;
	//send name 
	std::cout << "Name: ";
	char inputBuffer[MESSAGE_SIZE];
	std::cin.getline(inputBuffer, MESSAGE_SIZE);
	message.SetMessage(inputBuffer);
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	char OutBuffer[BUFFER_SIZE] = {};
	memcpy(OutBuffer, &message, BUFFER_SIZE);
	sendto(ClientSocket, OutBuffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddress, sizeof(serverAddress));

	//get id
	int serverAddressSize = sizeof(serverAddress);
	char InBuffer[BUFFER_SIZE] = {};
	int bytesIn = recvfrom(ClientSocket, InBuffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddress, &serverAddressSize);
	if (bytesIn == SOCKET_ERROR)
	{
		cout << "Error receiving from Server" << WSAGetLastError() << endl;
		return false;
	}

	if ((MessageType)InBuffer[0] == MessageType::eChatMessage)
	{
		ChatMessage* serverMessage = reinterpret_cast<ChatMessage*>(&InBuffer);
		std::cout << serverMessage->GetClientMessage() << std::endl;

		if ((MessageType)InBuffer[1] == MessageState::FirstSend)
		{
			globalID = serverMessage->GetClientID();
			std::cout << "Client have connected" << std::endl;
		}
	}



	std::thread SendTo(&SendToServer, ClientSocket, serverAddress);
	std::thread ReceiveFrom(&ResiveFromServer, ClientSocket, serverAddress);
	bool whileNotExit=true;
	while (whileNotExit)
	{
		SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	}
	// close socket 

	
	closesocket(ClientSocket);

	SendTo.join();
	ReceiveFrom.join();
	// shoutdown winsock
	WSACleanup();
	exit(-1);

}

bool SendToServer(const SOCKET& aServerScoket, const sockaddr_in& aServerAddress)
{

	ChatMessage message;
	while (true)
	{
		char inputBuffer[MESSAGE_SIZE];
		std::cin.getline(inputBuffer, MESSAGE_SIZE);
		message.SetMessage(inputBuffer);
		message.SetID(globalID);

		//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		//std::cin.clear();
		char OutBuffer[BUFFER_SIZE] = {};
		memcpy(OutBuffer, &message, BUFFER_SIZE);

		int bytesOut = sendto(aServerScoket, OutBuffer, BUFFER_SIZE, 0, (sockaddr*)&aServerAddress, sizeof(aServerAddress));
		if (bytesOut == SOCKET_ERROR)
		{
			cout << "Error Sending to server" << WSAGetLastError() << endl;
			return false;
		}
		ZeroMemory(OutBuffer, BUFFER_SIZE);
	}
	return false;
}

bool ResiveFromServer(const SOCKET& aServerScoket, const sockaddr_in& aServerAddress)
{

	int sreverLength = sizeof(aServerAddress);
	char InBuffer[BUFFER_SIZE] = {};
	while (true)
	{
		int bytesIn = recvfrom(aServerScoket, InBuffer, BUFFER_SIZE, 0, (sockaddr*)&aServerAddress, &sreverLength);
		if (bytesIn == SOCKET_ERROR)
		{
			cout << "Error receiving from Server" << WSAGetLastError() << endl;
			return false;
		}

		if ((MessageType)InBuffer[0] == MessageType::eChatMessage)
		{
			ChatMessage* serverMessage = reinterpret_cast<ChatMessage*>(&InBuffer);
			std::cout << endl;
			std::cout << serverMessage->GetClientMessage() << std::endl;
		}
		else
		{
			cout << "no message type in the buffer from the server" << WSAGetLastError() << endl;
		}
	}

}

BOOL __stdcall HandlerRoutine(DWORD dwCtrlType)
{
	if (dwCtrlType==CTRL_CLOSE_EVENT)
	{
		sockaddr_in serverAddress;
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(54000); // convert from little to big endian
		inet_pton(AF_INET, IP_ADDRESS, &serverAddress.sin_addr);
		ChatMessage message;
		char OutBuffer[BUFFER_SIZE] = {};
		SOCKET ClientSocket = socket(AF_INET, SOCK_DGRAM, 0);
		std::cout << "KEY IS DOWN " << std::endl;

		message.SetState(MessageState::LastSend);
		message.SetID(globalID);
		memcpy(OutBuffer, &message, BUFFER_SIZE);

		sendto(ClientSocket, OutBuffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddress, sizeof(serverAddress));
		return true;
	}
	return false;
}


//if (isFirstTime == false)
//{
//	isFirstTime = true;
//	std::cout << "Name: ";
//	char inputBuffer[MESSAGE_SIZE];
//	std::cin.getline(inputBuffer, MESSAGE_SIZE);
//	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//	std::cin.clear();
//	message.SetMessage(inputBuffer);		}
//else
//{
//	std::cout << "Input: ";
//	char inputBuffer[MESSAGE_SIZE];
//	//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//	std::cin.getline(inputBuffer, MESSAGE_SIZE);
//	//std::cin.clear();
//	message.SetMessage(inputBuffer);
//	message.GetClientMessage();
//}

//if (!SendToServer(ClientSocket, serverAddress, message))
//{
//	return 2;
//}


//if (!ResiveFromServer(ClientSocket, serverAddress, message))
//{
//	return 2;
//}
