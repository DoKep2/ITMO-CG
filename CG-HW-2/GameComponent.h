#pragma once

#include "Game.h"

class GameComponent {
public:
    Game* game;
    GameComponent(Game* g);
    virtual ~GameComponent();
	virtual void DestroyResources() = 0;
	virtual void Draw() = 0;
	virtual void Initialize() = 0;
	virtual void Reload() = 0;
	virtual void Update() = 0;
};
