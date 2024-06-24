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

private:
	bool isFisrtMsg = true;
	float moveSize = 1.f;


};