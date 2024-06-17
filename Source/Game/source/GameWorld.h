#pragma once

class GameWorld
{
public:
	GameWorld(); 
	~GameWorld();

	void Init();
	void Update(float aTimeDelta); 
	void Render();
	void IMGUIBOX();
	

private:
	bool isFisrtMsg = true;
	Tga::Sprite2DInstanceData myTGELogoInstance = {};
	Tga::SpriteSharedData sharedData = {};
};