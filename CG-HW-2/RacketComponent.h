#pragma once
#include "GameComponent.h"
#include "SimpleMath.h"
#include "ConstantBufferTypes.h"
#include "ConstantBuffer.h"

class RacketComponent : public GameComponent {
public:
    RacketComponent(Game* g);
    ~RacketComponent();

    void Initialize() override;
    void Reload() override;
    void Update() override;
    void Draw() override;
    void DestroyResources() override;

    void SetPosition(float x, float y);
    void MoveUp(float amount);
    void MoveDown(float amount);
    void MoveLeft(float amount);
    void MoveRight(float amount);
    void Rotate(float angle);
    void Rotate(float angle, float x, float y);

    DirectX::XMFLOAT3 GetPosition() const;
    DirectX::SimpleMath::Vector2 GetSize() const;
    DirectX::XMMATRIX GetMatrix() const;

    void UpdateRotation();


private:
    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;
    ID3D11InputLayout* layout_;
    ConstantBuffer<CB_VS_vertexshader> constantBuffer;
    DirectX::XMFLOAT4 points_[8];
    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;
    ID3D11RasterizerState* rastState_;
    ID3DBlob* vertexShaderByteCode_;
    ID3DBlob* pixelShaderByteCode_;
    UINT strides_[1];
    UINT offsets_[1];
    float rotationAngle = 0.0f;
    DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();
};