#include <iostream>

#include "TriangleComponent.h"
#include "RectangleComponent.h"
#include "SphereComponent.h"
#include "Camera.h"
#include "Keys.h"
#include "PlanetConstants.h"

#include "MyGame.h"

MyGame::MyGame() : Game(L"MyGame", 800, 800)
{
	auto sphere = new SphereComponent(this, L"..\\src\\textures\\sun.dds", PlanetRadius::Sun, 0.01f);
	Components.push_back(sphere);

	auto mercury = new SphereComponent(this, L"..\\src\\textures\\mercury.dds", PlanetRadius::Mercury, PlanetVelocity::Mercury);
	mercury->SetPosition(0.0f, 0.0f, -PlanetDistanceToSun::Mercury);
	Components.push_back(mercury);


	auto venus = new SphereComponent(this, L"..\\src\\textures\\venus_surface.dds", PlanetRadius::Venus, PlanetVelocity::Venus);
	venus->SetPosition(0.0f, 0.0f, -PlanetDistanceToSun::Venus);
	Components.push_back(venus);

	auto earth = new SphereComponent(this, L"..\\src\\textures\\earth.dds", PlanetRadius::Earth, PlanetVelocity::Earth);
	earth->SetPosition(0.0f, 0.0f, -PlanetDistanceToSun::Earth);
	Components.push_back(earth);

	auto mars = new SphereComponent(this, L"..\\src\\textures\\mars.dds", PlanetRadius::Mars, PlanetVelocity::Mars);
	mars->SetPosition(0.0f, 0.0f, -PlanetDistanceToSun::Mars);
	Components.push_back(mars);

	auto jupiter = new SphereComponent(this, L"..\\src\\textures\\mars.dds", PlanetRadius::Jupiter, PlanetVelocity::Jupiter);
	jupiter->SetPosition(0.0f, 0.0f, -PlanetDistanceToSun::Jupiter);
	Components.push_back(jupiter);

	//auto triangle = new TriangleComponent(this);
	//triangle->SetPosition(-0.5f, -0.5f);
	//Components.push_back(triangle);
	//auto triangle2 = new TriangleComponent(this);
	//triangle->SetPosition(-0.3f, -0.3f);
	//Components.push_back(triangle2);
	//auto sphere = new SphereComponent(this);
	//sphere->Initialize();
	//Components.push_back(sphere);
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
	static float cameraSpeed = 0.03f;
	for (auto c : Components)
	{
		c->Update();
	}
	static SphereComponent* sphere = static_cast<SphereComponent*> (Components[0]);
	static Camera camera = sphere->camera;

	if (InputDev->IsKeyDown(Keys::Up)) {
		camera.AdjustPosition(camera.GetForwardVector() * cameraSpeed);
	}

	if (InputDev->IsKeyDown(Keys::Left)) {
		camera.AdjustPosition(camera.GetLeftVector() * cameraSpeed);
	}

	if (InputDev->IsKeyDown(Keys::Down)) {
		camera.AdjustPosition(camera.GetBackwardVector() * cameraSpeed);
	}

	if (InputDev->IsKeyDown(Keys::Right)) {
		camera.AdjustPosition(camera.GetRightVector() * cameraSpeed);
	}

	if (InputDev->IsKeyDown(Keys::Space)) {
		camera.AdjustPosition(0.0f, cameraSpeed, 0.0f);
	}

	if (InputDev->IsKeyDown(Keys::Z)) {
		camera.AdjustPosition(0.0f, cameraSpeed, 0.0f);
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

	float angleRad = DirectX::XMConvertToRadians(angle);

	float rotatedTopLeftX = (topLeftX - pos.x) * cos(angleRad) - (topLeftY - pos.y) * sin(angleRad) + pos.x;
	float rotatedTopLeftY = (topLeftX - pos.x) * sin(angleRad) + (topLeftY - pos.y) * cos(angleRad) + pos.y;

	float rotatedTopRightX = (topRightX - pos.x) * cos(angleRad) - (topRightY - pos.y) * sin(angleRad) + pos.x;
	float rotatedTopRightY = (topRightX - pos.x) * sin(angleRad) + (topRightY - pos.y) * cos(angleRad) + pos.y;

	float rotatedCenterX = (rotatedTopLeftX + rotatedTopRightX) / 2;
	float rotatedCenterY = (rotatedTopLeftY + rotatedTopRightY) / 2;

	triangle->Initialize();
	triangle->SetPosition(rotatedCenterX, rotatedCenterY);
	triangle->SetRotation(angle);
	triangle->SetVelocity(0.01 * sin(angleRad), 0.01 * cos(angleRad));
}
