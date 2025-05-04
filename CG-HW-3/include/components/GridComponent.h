#pragma once

#include <ConstantBuffer.h>
#include <ConstantBufferTypes.h>

#include "GameComponent.h"
#include <DirectXMath.h>

using namespace DirectX;

struct GridVertex {
    XMFLOAT4 pos;
    XMFLOAT4 color;
};

class GridComponent : public GameComponent {
public:
    GridComponent(Game* game, int halfSize = 10, float spacing = 1.0f);
    ~GridComponent();

    void Initialize() override;
    void Draw() override;
    void DestroyResources() override;
    void Update() override;
    void Reload() override;

private:
    XMFLOAT4 GetColor(bool isAxis, bool isZAxis = true);

    GridVertex* vertices_;
    int pointsCount_;
    int halfSize_;
    float spacing_;

    ID3D11Buffer* vb_ = nullptr;
    ID3D11InputLayout* layout_ = nullptr;
    ID3D11VertexShader* vertexShader_ = nullptr;
    ID3D11PixelShader* pixelShader_ = nullptr;

    ConstantBuffer<CB_VS_vertexshader> constantBuffer;
};
