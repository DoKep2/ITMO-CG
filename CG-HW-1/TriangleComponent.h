//
// Created by sergo on 18.02.2025.
//
#pragma once

#include <directxmath.h>

#include "GameComponent.h"
#include "SimpleMath.h"
#include "ConstantBufferTypes.h"
#include "ConstantBuffer.h"
#include "SquareComponent.h"

class TriangleComponent : public GameComponent {
private:
    ID3D11InputLayout* layout_;
    ID3D11PixelShader* pixelShader_;
    ID3DBlob* pixelShaderByteCode_;
    DirectX::SimpleMath::Vector4 points_[6];
    ID3D11RasterizerState* rastState_;
    ID3D11VertexShader* vertexShader_;
    ID3DBlob* vertexShaderByteCode_;
    ConstantBuffer<CB_VS_vertexshader> constantBuffer;
    int indices_[6];
    DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();
    float rotationAngle = 0.0f;
    DirectX::SimpleMath::Vector2 velocity_;

    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;
    UINT strides_[1];
    UINT offsets_[1];

public:
    TriangleComponent(Game* g);
    ~TriangleComponent();
    void DestroyResources() override;
    void Draw() override;
    void Initialize() override;
    void Update() override;
    void Reload() override;
    
    DirectX::XMFLOAT3 GetPosition() const;

    DirectX::SimpleMath::Vector4* GetPoints();
    void SetPosition(float x, float y);
    void SetRotation(float angle);
    void SetVelocity(float x, float y);
    void CheckCollision(SquareComponent* square);
};


