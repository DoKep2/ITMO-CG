#include "PongGame.h"
#include "TriangleComponent.h"
#include "RectangleComponent.h"
#include "BallComponent.h"
#include "RacketComponent.h"
#include <random>
#include <string>
#include "d2d1.h"

static constexpr float kStartBallSpeed = 0.015f;

PongGame::PongGame() : Game(L"PongGame", 800, 800)
{
	firstPlayerRacket = new RacketComponent(this);
	secondPlayerRacket = new RacketComponent(this);
    firstPlayerRacket->SetPosition(-0.95f, 0.0f);
    secondPlayerRacket->SetPosition(0.92f, 0.0f);
	ball = new BallComponent(this);
	std::pair<float, float> dir = GenerateStartBallDirection();
	ball->SetVelocity(dir.first, dir.second);
	//ball->SetVelocity(0.015, -0.01);
	Components.push_back(ball);
	Components.push_back(firstPlayerRacket);
	Components.push_back(secondPlayerRacket);
}

void PongGame::Initialize()
{
    Game::Initialize();
        
    static const WCHAR msc_fontName[] = L"Verdana";
    static const FLOAT msc_fontSize = 50;
    HRESULT hr;

    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2DFactory);

    if (SUCCEEDED(hr))
    {
        // Create a DirectWrite factory.
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(DWFactory),
            reinterpret_cast<IUnknown**>(&DWFactory)
        );
    }
    if (SUCCEEDED(hr))
    {
        // Create a DirectWrite text format object.
        hr = DWFactory->CreateTextFormat(
            msc_fontName,
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            msc_fontSize,
            L"", //locale
            &DWTextFormat
        );
    }
    if (SUCCEEDED(hr))
    {
        // Center the text horizontally and vertically.
        DWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

        DWTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    hr = SwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&D2DBackBuff);

    // Create the DXGI Surface Render Target.
    float dpi = GetDpiForWindow(Display->hWnd);

    D2D1_RENDER_TARGET_PROPERTIES props =
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
            0.0f,
            0.0f);

    // Create a Direct2D render target that can draw into the surface in the swap chain

    hr = D2DFactory->CreateDxgiSurfaceRenderTarget(
        D2DBackBuff,
        &props,
        &D2DRenderTarget);

    hr = D2DRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White, 1.0f),
        &D2Dbrush
    );

    hr = D2DFactory->CreateStrokeStyle(
        D2D1::StrokeStyleProperties(
            D2D1_CAP_STYLE_FLAT,
            D2D1_CAP_STYLE_FLAT,
            D2D1_CAP_STYLE_ROUND,
            D2D1_LINE_JOIN_MITER,
            10.0f,
            D2D1_DASH_STYLE_DASH,
            0.0f),
        nullptr,
        0,
        &D2DLineStrokeStyle
    );
}

PongGame::~PongGame()
{
	DestroyResources();
}

void PongGame::DestroyResources()
{
	D2DLineStrokeStyle->Release();
	D2Dbrush->Release();
	D2DRenderTarget->Release();
	D2DBackBuff->Release();
	DWTextFormat->Release();
	DWFactory->Release();
	D2DFactory->Release();
}

void PongGame::Draw()
{
	float color[] = { 0, 0.0f, 0.0f, 1.0f };
	Context->ClearRenderTargetView(RenderView, color);

	for (auto c : Components)
	{
		c->Draw();
	}

    D2DRenderTarget->BeginDraw();
    std::wstring firstPlayerScoreStr = std::to_wstring(firstPlayerScore);
    std::wstring secondPlayerScoreStr = std::to_wstring(secondPlayerScore);

    D2DRenderTarget->DrawText(
        firstPlayerScoreStr.c_str(),
        firstPlayerScoreStr.size(),
        DWTextFormat,
        D2D1::RectF(0, 0.2 * Display->ClientHeight, 0.5f * Display->ClientWidth, 0),
		D2Dbrush
	);
	D2DRenderTarget->DrawText(
		secondPlayerScoreStr.c_str(),
		secondPlayerScoreStr.size(),
		DWTextFormat,
		D2D1::RectF(0.5f * Display->ClientWidth, 0.2 * Display->ClientHeight, Display->ClientWidth, 0),
		D2Dbrush
	);
	D2DRenderTarget->EndDraw();
}

void PongGame::Update() {
    for (auto& c : Components)
    {
        c->Update();
    }

    for (auto& c : ComponentsToAdd) {
		Components.push_back(c);
    }
	ComponentsToAdd.clear();

    if (this->InputDev->IsKeyDown(Keys::W)) {
        if (firstPlayerRacket->GetPosition().y + firstPlayerRacket->GetSize().y < 1.0f) {
            firstPlayerRacket->MoveUp(0.03);
        }
    }
    if (this->InputDev->IsKeyDown(Keys::S)) {
        if (firstPlayerRacket->GetPosition().y > -1.0f) {
            firstPlayerRacket->MoveDown(0.03f);
        }
    }

    #ifdef AI_PLAYERS
        auto racketPos = secondPlayerRacket->GetPosition();
        auto ballPos = ball->GetPosition();
        if (ballPos.y < racketPos.y && racketPos.y - secondPlayerRacket->GetSize().y - 0.015f > -1.0f) {
            secondPlayerRacket->MoveDown(0.015f);
        }
        else if (ballPos.y > racketPos.y && racketPos.y + secondPlayerRacket->GetSize().y + 0.015f < 1.0f) {
            secondPlayerRacket->MoveUp(0.015f);
        }
    #else
	    if (this->InputDev->IsKeyDown(Keys::Up)) {
		    if (secondPlayerRacket->GetPosition().y + secondPlayerRacket->GetSize().y < 1.0f) {
			    secondPlayerRacket->MoveUp(0.03f);
		    }
	    }
	    if (this->InputDev->IsKeyDown(Keys::Down)) {
		    if (secondPlayerRacket->GetPosition().y > -1.0f) {
			    secondPlayerRacket->MoveDown(0.03f);
		    }
	    }
    #endif
}

void PongGame::UpdateResult() {
    if (ball->GetPosition().x > 1.0f) {
        firstPlayerScore++;
    }
    else if (ball->GetPosition().x < -1.0f) {
        secondPlayerScore++;
    }
}

std::pair<float, float> PongGame::GenerateStartBallDirection()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 1);
	std::uniform_real_distribution<float> disY(-0.01f, 0.01f);
	return std::make_pair(dis(gen) == 0 ? -kStartBallSpeed : kStartBallSpeed, disY(gen));
}
