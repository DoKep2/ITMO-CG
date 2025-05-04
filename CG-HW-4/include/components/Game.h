#pragma once

#include <Camera.h>
#include <vector>
#include <wrl.h>
#include <d3d.h>
#include <d3d11.h>
#include <chrono>
#include "DisplayWin32.h"
#include "InputDevice.h"

class GameComponent;

class Game {
private:
    bool isExitRequested;

    void CreateBackBuffer();

protected:
    virtual void DestroyResources();

    virtual void Draw();

    virtual void EndFrame();

    virtual void Initialize();

    virtual void PrepareFrame();

    virtual void PrepareResources();

    virtual void Update();

    virtual void UpdateInternal();

public:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
    Game(LPCWSTR name, int screenWidth, int screenHeight);
    virtual ~Game();

    ID3D11Texture2D* backBuffer;
    ID3D11DeviceContext* Context;
    int DebugAnnotation;
    Microsoft::WRL::ComPtr<ID3D11Device> Device;
    HINSTANCE Instance;
    LPCWSTR Name;
    std::chrono::steady_clock::time_point PrevTime;
    ID3D11Texture2D* RenderSRV;
    ID3D11RenderTargetView* RenderView;
    ID3D11Texture2D* depthStencilBuffer;
    ID3D11DepthStencilView* depthStencilView;
	ID3D11DepthStencilState* depthStencilState;
    int ScreenResized;
    float StartTime;
    IDXGISwapChain* SwapChain;
    DisplayWin32* Display;
    InputDevice* InputDev;
    std::vector<GameComponent* > Components;

    Camera* camera;

    float TotalTime;

    void Exit();
    void MessageHandler();
    void RestoreTargets();
    void Run();
};
