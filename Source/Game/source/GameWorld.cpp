#include "stdafx.h"

#include "GameWorld.h"

#include <tge/graphics/GraphicsEngine.h>
#include <tge\engine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/texture/TextureManager.h>
#include <tge\input\InputManager.h>

//network
#include <tge/drawers/DebugDrawer.h>
#include <iostream>
#include <WS2tcpip.h>
#include <limits>
#include <thread>
#include "NetMessage.h"
#pragma comment (lib, "ws2_32.lib")

using namespace std;

#define BUFFER_SIZE 512
#define MESSAGE_SIZE 300
#define PORT 54000
#define IP_ADDRESS "127.0.0.1"

#ifdef max
#undef max
#endif

int globalID = -1;
std::string inputText = "";
std::vector<std::string> chatMessages;

std::array<Tga::Sprite2DInstanceData, 4>  myTGELogoInstance = {};
Tga::SpriteSharedData sharedData = {};


SOCKET ClientSocket;
sockaddr_in serverAddress;
bool ResiveFromServer(const SOCKET& aServerScoket, const sockaddr_in& aServerAddress);
int Main(const std::string& name);

BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType);
GameWorld::GameWorld() {}
GameWorld::~GameWorld() {}

int Main(const std::string& name)
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

	// Socket creation
	ClientSocket = socket(AF_INET, SOCK_DGRAM, 0);

	// A hint structure for the server

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(54000); // convert from little to big endian
	inet_pton(AF_INET, IP_ADDRESS, &serverAddress.sin_addr);

	ChatMessage message;

	message.SetMessage(name.c_str());
	char OutBuffer[BUFFER_SIZE] = {};
	memcpy(OutBuffer, &message, BUFFER_SIZE);
	sendto(ClientSocket, OutBuffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddress, sizeof(serverAddress));


	// Get ID
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
			for (const auto& transform: serverMessage->GetAllClinets())
			{
				myTGELogoInstance[transform.myID].myPosition = transform.myPosition;
			}
			std::cout << "Client have connected" << std::endl;
		}
	}

	std::thread ReceiveFrom(&ResiveFromServer, ClientSocket, serverAddress);
	bool whileNotExit = true;
	while (whileNotExit)
	{
		SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	}
	// close socket 

	closesocket(ClientSocket);
	ReceiveFrom.join();
	// shoutdown winsock
	WSACleanup();
	exit(-1);
}
void GameWorld::Init()
{
	auto& engine = *Tga::Engine::GetInstance();

	Tga::Vector2ui intResolution = engine.GetRenderSize();
	Tga::Vector2f resolution = { (float)intResolution.x, (float)intResolution.y };

	for (auto& instance : myTGELogoInstance)
	{
		sharedData.myTexture = engine.GetTextureManager().GetTexture(L"Sprites/tga_w.dds");

		instance.myPivot = { 0.5f, 0.5f };
		instance.myPosition = Tga::Vector2f{ 0.5f, 0.5f }*resolution;
		instance.mySize = Tga::Vector2f{ 0.75f, 0.75f }*resolution.y;

		instance.myColor = Tga::Color(1, 1, 1, 1);
	}

}
void GameWorld::Update(float aTimeDelta)
{
	UNREFERENCED_PARAMETER(aTimeDelta);
}

void GameWorld::Render(Tga::InputManager* aInput)
{
	if (AskForName() == false)
	{
		return;
	}

	auto& engine = *Tga::Engine::GetInstance();
	Tga::SpriteDrawer& spriteDrawer(engine.GetGraphicsEngine().GetSpriteDrawer());

	for (auto& instance : myTGELogoInstance)
	{
		spriteDrawer.Draw(sharedData, instance);
	};

	if (aInput->IsKeyPressed('W')|| aInput->IsKeyPressed('A') || aInput->IsKeyPressed('S') || aInput->IsKeyPressed('D'))
	{
		ChatMessage message;
		message.SetPosition(myTGELogoInstance[globalID].myPosition);
		message.SetID(globalID);
		message.ChangeMessageType(ePositsionMessage);

		char OutBuffer[BUFFER_SIZE] = {};
		memcpy(OutBuffer, &message, BUFFER_SIZE);
		sendto(ClientSocket, OutBuffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddress, sizeof(serverAddress));
	}

	

	if (aInput->IsKeyPressed(0x42))
	{
		std::cout << "working";
	}

	ImGui::Begin("Chat");
	for (const auto& message : chatMessages)
	{
		ImGui::TextWrapped("%s", message.c_str());
	}

	static char buf[MESSAGE_SIZE];
	if (ImGui::InputText("Input", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (buf[0] != '\0')
		{
			inputText = buf;
			ChatMessage message;
			message.SetMessage(buf);
			message.SetID(globalID);
			
			char OutBuffer[BUFFER_SIZE] = {};
			memcpy(OutBuffer, &message, BUFFER_SIZE);

			sendto(ClientSocket, OutBuffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddress, sizeof(serverAddress));

			buf[0] = '\0';
		}
	}
	ImGui::End();
}

bool GameWorld::AskForName()
{
	static bool isKlicked = false;
	static char name[128] = "";
	if (isKlicked)
	{
		return true;
	}
	ImGui::Begin("User Input");
	ImGui::InputTextWithHint("Name", "enter your name here", name, IM_ARRAYSIZE(name));
	if (ImGui::Button("Connect"))
	{
		if (name[0] != '\0')
		{
			isKlicked = true;
		}
	}
	ImGui::End();



	if (!isKlicked)
	{
		return false;
	}

	std::string inputStr(name);
	std::thread main([inputStr]() { Main(inputStr); });
	main.detach();

	return true;
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

			chatMessages.push_back(serverMessage->GetClientID() + serverMessage->GetClientMessage());
		}
		
		if ((MessageType)InBuffer[0] == MessageType::ePositsionMessage)
		{
			ChatMessage* serverMessage = reinterpret_cast<ChatMessage*>(&InBuffer);
			std::cout << "ID: " << serverMessage->GetClientID() << ", x: " << serverMessage->GetPosition().myX << ", y: " << serverMessage->GetPosition().myY << std::endl;
		}

		else
		{
			cout << "no message type in the buffer from the server" << WSAGetLastError() << endl;
		}
	}
}

BOOL __stdcall HandlerRoutine(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT)
	{
		serverAddress;
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(54000); // convert from little to big endian
		inet_pton(AF_INET, IP_ADDRESS, &serverAddress.sin_addr);
		ChatMessage message;
		char OutBuffer[BUFFER_SIZE] = {};
		ClientSocket = socket(AF_INET, SOCK_DGRAM, 0);
		std::cout << "KEY IS DOWN " << std::endl;

		message.SetState(MessageState::LastSend);
		message.SetID(globalID);
		memcpy(OutBuffer, &message, BUFFER_SIZE);

		sendto(ClientSocket, OutBuffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddress, sizeof(serverAddress));
		return true;
	}
	return false;
}
