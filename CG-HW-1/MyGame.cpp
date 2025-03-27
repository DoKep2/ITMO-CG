#include "MyGame.h"
#include "TriangleComponent.h"
#include "RectangleComponent.h"
#include "SquareComponent.h"
#include <iostream>
#include <random>

MyGame::MyGame() : Game(L"MyGame", 800, 800)
{
	auto cannon = new RectangleComponent(this);
	cannon->SetPosition(0, -0.8f);
	Components.push_back(cannon);
	auto square = new SquareComponent(this);
	// // generate random position
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.0f, 0.8f);
	square->SetPosition(dis(gen), dis(gen));
	Components.push_back(square);

	// auto triange = new TriangleComponent(this);
	// auto square = new SquareComponent(this);
	// Components.push_back(triange);
	// Components.push_back(square);
}

MyGame::~MyGame()
{
}

void MyGame::Draw()
{
	float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Context->ClearRenderTargetView(RenderView, color);
	
	for (auto c : Components)
	{
		c->Draw();
	}
}


void MyGame::Update() {
	static bool upPressed = false;

	for (auto c : Components)
	{
		c->Update();
	}

	for (auto& c : ComponentsToAdd) {
		Components.push_back(c);
	}
	ComponentsToAdd.clear();

	if (InputDev->IsKeyDown(Keys::Left)) {
		dynamic_cast<RectangleComponent*> (Components[0])->SetRotation(DirectX::XMConvertToRadians(1));
	}

	if (InputDev->IsKeyDown(Keys::Right)) {
		dynamic_cast<RectangleComponent*>(Components[0])->SetRotation(-DirectX::XMConvertToRadians(1));
	}

	if (InputDev->IsKeyDown(Keys::Up)) {
		if (!upPressed) {
			const auto cannon = dynamic_cast<RectangleComponent*>(Components[0]);
			const auto triangle = new TriangleComponent(this);
			triangle->Initialize();
			BindTriangleToCannon(triangle, cannon);
			Components.push_back(triangle);
			upPressed = true;
		}
	}
	else {
		upPressed = false;
	}
}

void MyGame::BindTriangleToCannon(TriangleComponent* triangle, RectangleComponent* cannon)
{
	auto pos = cannon->GetPosition();
	float x = 0.025;
	float y = 0.15;
	float angle = cannon->GetRotation();

	float topLeftX = pos.x;
	float topLeftY = pos.y + y;
	float topRightX = pos.x + x;
	float topRightY = pos.y + y;

	float angleRad = angle;

	float rotatedTopLeftX = (topLeftX - pos.x) * cos(angleRad) - (topLeftY - pos.y) * sin(angleRad) + pos.x;
	float rotatedTopLeftY = (topLeftX - pos.x) * sin(angleRad) + (topLeftY - pos.y) * cos(angleRad) + pos.y;

	float rotatedTopRightX = (topRightX - pos.x) * cos(angleRad) - (topRightY - pos.y) * sin(angleRad) + pos.x;
	float rotatedTopRightY = (topRightX - pos.x) * sin(angleRad) + (topRightY - pos.y) * cos(angleRad) + pos.y;

	float rotatedCenterX = (rotatedTopLeftX + rotatedTopRightX) / 2;
	float rotatedCenterY = (rotatedTopLeftY + rotatedTopRightY) / 2;

	triangle->SetPosition(rotatedCenterX, rotatedCenterY);
	auto trianglePos = triangle->GetPosition();
	triangle->SetRotation(angle, trianglePos.x, trianglePos.y);
	triangle->SetVelocity(0.01f * cos(angleRad + DirectX::XM_PIDIV2), 0.01f * sin(angleRad + DirectX::XM_PIDIV2));
}
