#pragma once
#include <string>

enum MessageType :unsigned char
{
	eNone,
	eChatMessage,
	eInfoMessage
};


enum MessageState :unsigned char
{
	FirstSend,
	LastSend,
	None,
};


class NetMessage
{
protected:
	MessageType myType = MessageType::eNone;
	MessageState myState = MessageState::None;
	NetMessage() = default;
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
	void SetState(MessageState aState) { myState = aState; };
	const char* GetClientMessage() const { return myMessage; };
	int GetClientID() { return myID; };
private:
	int myID = -1;
	char myMessage[512] = {};
};
