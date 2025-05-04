#include <iostream>

#include "TriangleComponent.h"
#include "RectangleComponent.h"
#include "SphereComponent.h"
#include "Camera.h"
#include "Keys.h"
#include "PlanetConstants.h"

#include "MyGame.h"

#include "OrbitComponent.h"

MyGame::MyGame() : Game(L"MyGame", 800, 800)
{
	// Создание орбиты

	auto sun = new SphereComponent(this, L"..\\src\\textures\\sun.dds", PlanetRadius::Sun);
	sun->SetOrbitingTarget(sun, PlanetRadius::Sun, PlanetVelocity::Mercury);
	Components.push_back(sun);

	auto mercury = new SphereComponent(this, L"..\\src\\textures\\mercury.dds", PlanetRadius::Mercury);
	mercury->SetPosition(XMFLOAT3(0.0f, 0.0f, -PlanetDistanceToSun::Mercury));
	mercury->SetOrbitingTarget(sun, PlanetRadius::Mercury, PlanetVelocity::Mercury * 100);
	Components.push_back(mercury);

	auto venus = new SphereComponent(this, L"..\\src\\textures\\venus_surface.dds", PlanetRadius::Venus);
	venus->SetPosition(XMFLOAT3(0.0f, 0.0f, -PlanetDistanceToSun::Venus));
	venus->SetOrbitingTarget(sun, PlanetRadius::Venus, PlanetVelocity::Venus * 100);
	Components.push_back(venus);

	auto earth = new SphereComponent(this, L"..\\src\\textures\\earth.dds", PlanetRadius::Earth);
	earth->SetPosition(XMFLOAT3(0.0f, 0.0f, -PlanetDistanceToSun::Earth));
	earth->SetOrbitingTarget(sun, PlanetRadius::Earth, PlanetVelocity::Earth * 500);
	Components.push_back(earth);

	auto mars = new SphereComponent(this, L"..\\src\\textures\\mars.dds", PlanetRadius::Mars);
	mars->SetPosition(XMFLOAT3(0.0f, 0.0f, -PlanetDistanceToSun::Mars));
	mars->SetOrbitingTarget(sun, PlanetRadius::Mars, PlanetVelocity::Mars * 100);
	Components.push_back(mars);

	auto jupiter = new SphereComponent(this, L"..\\src\\textures\\mars.dds", PlanetRadius::Jupiter);
	jupiter->SetPosition(XMFLOAT3(0.0f, 0.0f, -PlanetDistanceToSun::Jupiter));
	jupiter->SetOrbitingTarget(sun, PlanetRadius::Jupiter, PlanetVelocity::Jupiter * 100);
	Components.push_back(jupiter);

	auto moon = new SphereComponent(this, L"..\\src\\textures\\moon.dds", PlanetRadius::Moon);
	moon->SetPosition(XMFLOAT3(0.0f, 0.0f, -PlanetDistanceToSun::Earth - 0.5));
	moon->SetOrbitingTarget(earth, PlanetRadius::Moon, PlanetVelocity::Moon * 1000);
	Components.push_back(moon);

	auto* mercuryOrbit = new OrbitComponent(this, PlanetDistanceToSun::Mercury, 300);
	mercuryOrbit->SetCenter(sun);

	Components.push_back(mercuryOrbit);

	auto* venusOrbit = new OrbitComponent(this, PlanetDistanceToSun::Venus, 300);
	venusOrbit->SetCenter(sun);

	Components.push_back(venusOrbit);

	auto* earthOrbit = new OrbitComponent(this, PlanetDistanceToSun::Earth, 300);
	earthOrbit->SetCenter(sun);

	Components.push_back(earthOrbit);

	auto* marsOrbit = new OrbitComponent(this, PlanetDistanceToSun::Mars, 300);
	marsOrbit->SetCenter(sun);

	Components.push_back(marsOrbit);

	auto* jupiterOrbit = new OrbitComponent(this, PlanetDistanceToSun::Jupiter, 300);
	jupiterOrbit->SetCenter(sun);

	Components.push_back(jupiterOrbit);

	auto* moonOrbit = new OrbitComponent(this, 0.5, 300);
	moonOrbit->SetCenter(earth);

	Components.push_back(moonOrbit);
}

MyGame::~MyGame()
{
}

void MyGame::Draw()
{
	float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Context->ClearRenderTargetView(RenderView, color);
	Context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

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
