#include "Game.h"

#include <GameComponent.h>
#include <iostream>

LRESULT CALLBACK Game::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	Game* pThis;

	if (umessage == WM_NCCREATE)
	{
		pThis = static_cast<Game*>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);

		SetLastError(0);
		if (!SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis)))
		{
			if (GetLastError() != 0)
				return FALSE;
		}
	}
	else
	{
		pThis = reinterpret_cast<Game*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	switch (umessage)
	{
	case WM_KEYDOWN:
	{
		if (static_cast<unsigned int>(wparam) == 27) PostQuitMessage(0);
		return 0;
	}
	case WM_INPUT:
	{
		UINT dwSize = 0;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == nullptr) {
			return 0;
		}

		if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

		auto* raw = reinterpret_cast<RAWINPUT*>(lpb);

		if (pThis)
		{
			if (raw->header.dwType == RIM_TYPEKEYBOARD)
			{
				//printf(" Kbd: make=%04i Flags:%04i Reserved:%04i ExtraInformation:%08i, msg=%04i VK=%i \n",
				//	raw->data.keyboard.MakeCode,
				//	raw->data.keyboard.Flags,
				//	raw->data.keyboard.Reserved,
				//	raw->data.keyboard.ExtraInformation,
				//	raw->data.keyboard.Message,
				//	raw->data.keyboard.VKey);
				pThis->InputDev->OnKeyDown({
					raw->data.keyboard.MakeCode,
					raw->data.keyboard.Flags,
					raw->data.keyboard.VKey,
					raw->data.keyboard.Message
					});
			}
			else if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				//printf(" Mouse: X=%04d Y:%04d \n", raw->data.mouse.lLastX, raw->data.mouse.lLastY);
				pThis->InputDev->OnMouseMove({
					raw->data.mouse.usFlags,
					raw->data.mouse.usButtonFlags,
					static_cast<int>(raw->data.mouse.ulExtraInformation),
					static_cast<int>(raw->data.mouse.ulRawButtons),
					static_cast<short>(raw->data.mouse.usButtonData),
					raw->data.mouse.lLastX,
					raw->data.mouse.lLastY
					});
			}
		}

		delete[] lpb;
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
	default:
	{
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
	}
}

void Game::CreateBackBuffer()
{
	auto res = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);	// __uuidof(ID3D11Texture2D)
	res = Device->CreateRenderTargetView(backBuffer, nullptr, &RenderView);
}

Game::Game(LPCWSTR name, int screenWidth, int screenHeight) : Name(name)
{
	Instance = GetModuleHandle(nullptr);

	Display = new DisplayWin32(name, Instance, screenWidth, screenHeight, this);
	InputDev = new InputDevice(this);
}

Game::~Game()
{
	for (auto c : Components)
	{
		delete c;
	}

	delete Display;
	delete InputDev;

	Context->Release();
	backBuffer->Release();
	RenderView->Release();
	SwapChain->Release();
}

void Game::Exit()
{
	DestroyResources();
}

void Game::MessageHandler()
{
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Game::RestoreTargets()
{
}

void Game::Run()
{
	PrepareResources();
	Initialize();
	isExitRequested = false;
	std::chrono::time_point<std::chrono::steady_clock> PrevTime = std::chrono::steady_clock::now();

	unsigned int frameCount = 0;
	while (!isExitRequested)
	{
		MessageHandler();

		auto curTime = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
		PrevTime = curTime;

		TotalTime += deltaTime;
		frameCount++;

		if (TotalTime > 1.0f) {
			float fps = frameCount / TotalTime;

			TotalTime -= 1.0f;

			WCHAR text[256];
			swprintf_s(text, L"%s - FPS: %f", Name, fps);
			// SetWindowText(Display->hWnd, reinterpret_cast<LPCSTR>(text));
			SetWindowText(Display->hWnd, text);

			frameCount = 0;
		}

		Context->ClearState();
		Context->OMSetRenderTargets(1, &RenderView, nullptr);

		Update();

		Draw();

		Context->OMSetRenderTargets(0, nullptr, nullptr);

		SwapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
	}
	Exit();
}

void Game::DestroyResources()
{
	for (auto c : Components)
	{
		c->DestroyResources();
	}

	Context->Release();
	backBuffer->Release();
	RenderView->Release();
	SwapChain->Release();
	depthStencilState->Release();
	depthStencilBuffer->Release();
	depthStencilView->Release();
}

void Game::Draw()
{
	float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Context->ClearRenderTargetView(RenderView, color);
	Context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	for (auto c : Components)
	{
		c->Draw();
	}
}

void Game::EndFrame()
{
}

void Game::Initialize()
{
	for (auto c : Components)
	{
		c->Initialize();
	}
}

void Game::PrepareFrame()
{
}

void Game::PrepareResources()
{
	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2;
	swapDesc.BufferDesc.Width = Display->ClientWidth;
	swapDesc.BufferDesc.Height = Display->ClientHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = Display->hWnd;
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	auto res = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapDesc,
		&SwapChain,
		&Device,
		nullptr,
		&Context);

	if (FAILED(res))
	{
		// Well, that was unexpected
	}

	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = Display->ClientWidth;
	depthDesc.Height = Display->ClientHeight;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;  // Тип глубины
	depthDesc.SampleDesc.Count = 1;  // Без антиалиасинга
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	// Создаем текстуру глубины
	HRESULT hr = Device->CreateTexture2D(&depthDesc, nullptr, &depthStencilBuffer);
	if (FAILED(hr)) {
		// Обработка ошибки
	}

	// Создание DepthStencilView для работы с этим буфером
	D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc = {};
	depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthViewDesc.Texture2D.MipSlice = 0;

	hr = Device->CreateDepthStencilView(depthStencilBuffer, &depthViewDesc, &depthStencilView);
	if (FAILED(hr)) {
		// Обработка ошибки
	}

	// Создание DepthStencilState для настройки поведения при рендеринге
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;  // Меньше или равно для правильной прорисовки

	hr = Device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	if (FAILED(hr)) {
		// Обработка ошибки
	}
	Context->OMSetDepthStencilState(depthStencilState, 0);
	CreateBackBuffer();
}


void Game::Update()
{
	for (auto c : Components)
	{
		c->Update();
	}
}

void Game::UpdateInternal()
{
}
