#include <iostream>

#include "TriangleComponent.h"
#include "RectangleComponent.h"
#include "SphereComponent.h"
#include "Camera.h"
#include "Keys.h"
#include "PlanetConstants.h"

#include "MyGame.h"

#include "Aboba.h"
#include "PlaneComponent.h"

MyGame::MyGame() : Game(L"MyGame", 800, 800)
{
	auto plane = new PlaneComponent(this, L"..\\src\\textures\\earth.dds", 1000.0f, 1000.0f);
	Components.push_back(plane);
	auto sphere = new SphereComponent(this);
	sphere->SetPosition(XMFLOAT3(0, 0.55, 0));
	Components.push_back(sphere);
	auto gift = new Aboba(this, L"..\\src\\textures\\cat.obj");
	gift->SetPosition(XMFLOAT3(2.0, -0.6, 2.0));
	gift->SetScale(XMFLOAT3(3.0f, 3.0f, 3.0f));
	Components.push_back(gift);

	auto gift2 = new Aboba(this, L"..\\src\\textures\\cat.obj");
	gift2->SetPosition(XMFLOAT3(-2.0, -0.6, 2.0));
	gift2->SetScale(XMFLOAT3(3.0f, 3.0f, 3.0f));
	Components.push_back(gift2);

	auto gift3 = new Aboba(this, L"..\\src\\textures\\gift.obj");
	gift3->SetPosition(XMFLOAT3(0.0, -0.6, 5.0));
	//gift3->SetScale(XMFLOAT3(3.0f, 3.0f, 3.0f));
	Components.push_back(gift3);
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

	if (InputDev->IsKeyDown(Keys::Up)) {
		camera->AdjustPosition(camera->GetForwardVector() * cameraSpeed);
	}

	if (InputDev->IsKeyDown(Keys::Left)) {
		camera->AdjustPosition(camera->GetLeftVector() * cameraSpeed);
	}

	if (InputDev->IsKeyDown(Keys::Down)) {
		camera->AdjustPosition(camera->GetBackwardVector() * cameraSpeed);
	}

	if (InputDev->IsKeyDown(Keys::Right)) {
		camera->AdjustPosition(camera->GetRightVector() * cameraSpeed);
	}

	if (InputDev->IsKeyDown(Keys::Space)) {
		camera->AdjustPosition(0.0f, cameraSpeed, 0.0f);
	}

	if (InputDev->IsKeyDown(Keys::LeftShift)) {
		camera->AdjustPosition(0.0f, -cameraSpeed, 0.0f);
	}

	if (InputDev->IsKeyDown(Keys::I)) {
		camera->AdjustRotation(-cameraSpeed, 0.0f, 0.0f);
	}
	if (InputDev->IsKeyDown(Keys::J)) {
		camera->AdjustRotation(0.0f, -cameraSpeed, 0.0f);
	}

	if (InputDev->IsKeyDown(Keys::K)) {
		camera->AdjustRotation(cameraSpeed, 0.0f, 0.0f);
	}

	if (InputDev->IsKeyDown(Keys::L)) {
		camera->AdjustRotation(0.0f, cameraSpeed, 0.0f);
	}
}
