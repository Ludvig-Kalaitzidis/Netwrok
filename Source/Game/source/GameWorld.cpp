#include "stdafx.h"
#include "GameWorld.h"
#include <tge/graphics/GraphicsEngine.h>
#include <tge/drawers/SpriteDrawer.h>
#include <tge/texture/TextureManager.h>
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
SOCKET ClientSocket;
sockaddr_in serverAddress;
bool SendToServer(const SOCKET& aServerScoket, const sockaddr_in& aServerAddress);
bool ResiveFromServer(const SOCKET& aServerScoket, const sockaddr_in& aServerAddress);
int Main();

BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType);
GameWorld::GameWorld() {}
GameWorld::~GameWorld() {}

int Main()
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
    // Send name
    // Replace console input with ImGui input
   

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
    {
        sharedData.myTexture = engine.GetTextureManager().GetTexture(L"Sprites/tge_logo_w.dds");

        myTGELogoInstance.myPivot = { 0.5f, 0.5f };
        myTGELogoInstance.myPosition = Tga::Vector2f{ 0.5f, 0.5f }*resolution;
        myTGELogoInstance.mySize = Tga::Vector2f{ 0.75f, 0.75f }*resolution.y;

        myTGELogoInstance.myColor = Tga::Color(1, 1, 1, 1);
    }

    std::thread main(&Main);
    main.detach();
}

void GameWorld::Update(float aTimeDelta)
{
    aTimeDelta;

}

void GameWorld::Render()
{
    auto& engine = *Tga::Engine::GetInstance();
    Tga::SpriteDrawer& spriteDrawer(engine.GetGraphicsEngine().GetSpriteDrawer());
    // Game update
    {
        spriteDrawer.Draw(sharedData, myTGELogoInstance);
    }

    // Debug draw pivot
#ifndef _RETAIL
    {
        Tga::DebugDrawer& dbg = engine.GetDebugDrawer();
        Tga::Color c1 = myTGELogoInstance.myColor;
        dbg.DrawCircle(myTGELogoInstance.myPosition, 5.f, (c1.myR + c1.myG + c1.myB) / 3 > 0.3f ? Tga::Color(0, 0, 0, 1) : Tga::Color(1, 1, 1, 1));
    }
#endif

    // ImGui Chatbox
    ImGui::Begin("Chat");
    for (const auto& message : chatMessages)
    {
        ImGui::TextWrapped("%s", message.c_str());
    }

    static char buf[MESSAGE_SIZE];
    if (ImGui::InputText("Input", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        if (buf[0])
        {
            inputText = buf;
            ChatMessage message;
            message.SetMessage(buf);
            message.SetID(globalID);
            char OutBuffer[BUFFER_SIZE] = {};
            memcpy(OutBuffer, &message, BUFFER_SIZE);

            // Send message to server
            // Assuming you have the ClientSocket and serverAddress variables accessible here
            sendto(ClientSocket, OutBuffer, BUFFER_SIZE, 0, (sockaddr*)&serverAddress, sizeof(serverAddress));

            // Clear the input buffer
            buf[0] = '\0';
        }
    }
    ImGui::End();
}

bool SendToServer(const SOCKET& aServerScoket, const sockaddr_in& aServerAddress)
{
    ChatMessage message;
    while (true)
    {
        if (!inputText.empty())
        {
            message.SetMessage(inputText.c_str());
            message.SetID(globalID);

            char OutBuffer[BUFFER_SIZE] = {};
            memcpy(OutBuffer, &message, BUFFER_SIZE);

            int bytesOut = sendto(aServerScoket, OutBuffer, BUFFER_SIZE, 0, (sockaddr*)&aServerAddress, sizeof(aServerAddress));
            if (bytesOut == SOCKET_ERROR)
            {
                cout << "Error Sending to server" << WSAGetLastError() << endl;
                return false;
            }
            ZeroMemory(OutBuffer, BUFFER_SIZE);
            inputText.clear();
        }
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
          
            chatMessages.push_back(serverMessage->GetClientID() + serverMessage->GetClientMessage());
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
