#pragma once

#include "Game.h"
#include "RacketComponent.h"
#include "BallComponent.h"
#include <d2d1.h>
#include <dwrite.h>

#define AI_PLAYER

class PongGame : public Game
{
protected:
	void Update() override;
	ID2D1Factory* D2DFactory;
	IDWriteFactory* DWFactory;
	IDWriteTextFormat* DWTextFormat;
	ID2D1RenderTarget* D2DRenderTarget;
	ID2D1SolidColorBrush* D2Dbrush;
	ID2D1StrokeStyle* D2DLineStrokeStyle;
	IDXGISurface* D2DBackBuff;
public:
	PongGame();
	~PongGame();
	void Initialize() override;
	void Draw() override;
	void DestroyResources() override;

	void UpdateResult();
	std::pair<float, float> GenerateStartBallDirection();

	int firstPlayerScore = 0;
	int secondPlayerScore = 0;
	RacketComponent* firstPlayerRacket;
	RacketComponent* secondPlayerRacket;
	BallComponent* ball;
};