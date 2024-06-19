#pragma once
#include <string>
#include <tge\math\Vector.h>
enum MessageType :unsigned char
{
	eNone,
	eChatMessage,
	eInfoMessage,
	ePositsionMessage
};


enum MessageState :unsigned char
{
	FirstSend,
	LastSend,
	None
};


class NetMessage
{
protected:
	MessageType myType = MessageType::eNone;
	MessageState myState = MessageState::None;
	NetMessage() = default;
};


class FirstMessage : public NetMessage
{
public:
	void SetConnectedClients(std::unordered_map<int, ClientInfo> allClients)
	{
		connectedClients = allClients;
	}
private:
	std::unordered_map<int, ClientInfo> connectedClients;
};

class ChatMessage : public NetMessage
{
public:
	ChatMessage() { myType = MessageType::eChatMessage; myID = -1; };
	~ChatMessage() = default;

	void SetMessage(const char* aMessage)
	{
		strncpy_s(myMessage, sizeof(myMessage), aMessage, _TRUNCATE);
		myMessage[sizeof(myMessage) - 1] = '\0';
	};
	void SetID(const int anID)
	{
		myID = anID;
	};
	void SetPosition(const Tga::Vector2f& aPosition)
	{
		positsion = aPosition;
	}
	void SetState(MessageState aState) { myState = aState; };
	void ChangeMessageType(MessageType aMessageType) { myType = aMessageType; };
	const char* GetClientMessage() const { return myMessage; };
	int GetClientID() { return myID; };
	Tga::Vector2f GetPosition() {return positsion;}
private:
	int myID = -1;
	char myMessage[256] = {};
	Tga::Vector2f positsion;
};
