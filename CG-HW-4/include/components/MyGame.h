#pragma once

#include "Game.h"
#include "TriangleComponent.h"
#include "RectangleComponent.h"

class MyGame : public Game
{
protected:
	void Draw() override;
	void Update() override;
public:
	MyGame();
	~MyGame();
};