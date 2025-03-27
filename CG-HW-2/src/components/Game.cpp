#include "Game.h"
#include "InputDevice.h"
#include <iostream>

LRESULT CALLBACK Game::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	Game* game;
	if (umessage == WM_NCCREATE)
	{
		game = static_cast<Game*>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);

		SetLastError(0);
		if (!SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(game)))
		{
			if (GetLastError() != 0)
				return FALSE;
		}
	}
	else
	{
		game = reinterpret_cast<Game*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	switch (umessage)
	{
	case WM_KEYDOWN:
	{
		if (wparam == 27)
			PostQuitMessage(0);
		return 0;
	}
	/*case WM_KEYUP:
	{
		return 0;
	}*/
	case WM_INPUT:
	{
		UINT dwSize = 0;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == nullptr) {
			return 0;
		}

		if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

		RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb);
		
		if (game) {
			if (raw->header.dwType == RIM_TYPEKEYBOARD)
			{
				game->InputDev->OnKeyDown({
					raw->data.keyboard.MakeCode,
					raw->data.keyboard.Flags,
					raw->data.keyboard.VKey,
					raw->data.keyboard.Message
					});
			}
			else if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				//printf(" Mouse: X=%04d Y:%04d \n", raw->data.mouse.lLastX, raw->data.mouse.lLastY);
				game->InputDev->OnMouseMove({
					raw->data.mouse.usFlags,
					raw->data.mouse.usButtonFlags,
					static_cast<int>(raw->data.mouse.ulExtraInformation),
					static_cast<int>(raw->data.mouse.ulRawButtons),
					static_cast<short>(raw->data.mouse.usButtonData),
					static_cast<short>(raw->data.mouse.lLastX),
					static_cast<short>(raw->data.mouse.lLastY)
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

void Game::CreateBackBuffer()
{
	auto res = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);	// __uuidof(ID3D11Texture2D)
	res = Device->CreateRenderTargetView(backBuffer, nullptr, &RenderView);
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

	if (msg.message == WM_KEYDOWN)
	{
		if (msg.wParam == 27)
		{
			isExitRequested = true;
			return;
		}
		InputDev->AddPressedKey(Keys(msg.wParam));
		return;
	}

	if (msg.message == WM_KEYUP)
	{
		InputDev->RemovePressedKey(Keys(msg.wParam));
		return;
	}

	if (msg.message == WM_INPUT)
	{
		UINT dwSize;

		GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == NULL)
		{
			return;
		}

		if (GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

		RAWINPUT* raw = (RAWINPUT*)lpb;

		if (raw->header.dwType == RIM_TYPEKEYBOARD)
		{
			InputDev->AddPressedKey(Keys(raw->data.keyboard.MakeCode));
			std::cout << raw->data.keyboard.MakeCode << "\n";
		}
		else if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			//InputDev->OnRawDelta(raw->data.mouse.lLastX, InputDev->MousePosY = raw->data.mouse.lLastY);
		}
		delete[] lpb;
		return;
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
}

void Game::Draw()
{
	float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Context->ClearRenderTargetView(RenderView, color);

	for (auto c : Components)
	{
		c->Draw();
	}
}

void Game::EndFrame()
{
	// ???
	Context->OMSetRenderTargets(0, nullptr, nullptr);

	SwapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
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
	// ???
	Context->ClearState();

	Context->OMSetRenderTargets(1, &RenderView, nullptr);

	constexpr float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Context->ClearRenderTargetView(RenderView, color);
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
		D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
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
