//
// Created by sergo on 18.02.2025.
//
#pragma once

#include <directxmath.h>

#include "GameComponent.h"
#include "SimpleMath.h"

class TriangleComponent : public GameComponent {
private:
    ID3D11InputLayout* layout_;
    ID3D11PixelShader* pixelShader_;
    ID3DBlob* pixelShaderByteCode_; // ???
    DirectX::XMFLOAT4 points_[8];
    //DirectX::XMFLOAT4 points_[16];
    ID3D11RasterizerState* rastState_;
    ID3D11VertexShader* vertexShader_;
    ID3DBlob* vertexShaderByteCode_; // ???
    int indices_[6];
    DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();

    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;
    UINT strides_[1];
    UINT offsets_[1];

    ID3D11Buffer* constBuffer;
public:
    TriangleComponent(Game* g);
    ~TriangleComponent();
    void DestroyResources() override;
    void Draw() override;
    void Initialize() override;
    void Update() override;
    void Reload() override;
    void MoveX(float x);
    void MoveY(float y);
};


