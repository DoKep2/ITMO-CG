//
// Created by sergo on 18.02.2025.
//
#pragma once

#include <d3d11.h>
#include <directxmath.h>

#include "GameComponent.h"
#include "SimpleMath.h"
#include "Vertex.h"
#include "ConstantBufferTypes.h"
#include "ConstantBuffer.h"
#include "Game.h"

class TriangleComponent : public GameComponent {
private:
    ID3D11InputLayout* layout_;
    ID3D11PixelShader* pixelShader_;
    ID3DBlob* pixelShaderByteCode_;
    DirectX::XMFLOAT4 points_[6];
    //Vertex points_[3]; 
    ID3D11RasterizerState* rastState_;
    ID3D11VertexShader* vertexShader_;
    ID3DBlob* vertexShaderByteCode_;
    ConstantBuffer<CB_VS_vertexshader> constantBuffer;
    int indices_[6];
    DirectX::SimpleMath::Vector4 offset;
    DirectX::SimpleMath::Matrix mat;
    float rotationAngle;
    DirectX::SimpleMath::Vector2 velocity_;
    DirectX::SimpleMath::Vector2 position_;

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
    
    DirectX::SimpleMath::Vector4 GetPosition() const;
    void SetPosition(float x, float y);
    void SetRotation(float angle);
    void SetVelocity(float x, float y);
};


