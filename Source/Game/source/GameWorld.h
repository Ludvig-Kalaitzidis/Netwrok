#pragma once
#include <tge\input\InputManager.h>
class GameWorld
{
public:
	GameWorld(); 
	~GameWorld();

	void Init();
	void Update(float aTimeDelta); 
	void Render(Tga::InputManager* aInput);
	bool AskForName();
	void IMGUIBOX();
	bool ResiveFromServer(const SOCKET& aServerScoket, const sockaddr_in& aServerAddress);
	int Main(const std::string& name);
private:
	bool isFisrtMsg = true;

	std::vector<Tga::Sprite2DInstanceData> myOtherClientsSprites = {};
	Tga::Sprite2DInstanceData mySpriteInstance = {};
	Tga::SpriteSharedData sharedData = {};

};