#pragma once

#include <vector>
#include <d3d.h>
#include <wrl.h>
#include <d3d11.h>
#include <chrono>
#include "DisplayWin32.h"
#include "InputDevice.h"
#include "GameComponent.h"

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
    int ScreenResized;
    float StartTime;
    IDXGISwapChain* SwapChain;
    DisplayWin32* Display;
    InputDevice* InputDev;
    std::vector<GameComponent * > Components;
    std::vector<GameComponent*> ComponentsToAdd;

    float TotalTime;

    void Exit();
    void MessageHandler();
    void RestoreTargets();
    void Run();
};
