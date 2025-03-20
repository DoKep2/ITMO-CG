#pragma once
#include "GameComponent.h"
#include "SimpleMath.h"

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

    DirectX::SimpleMath::Vector2 GetPosition() const;
    DirectX::SimpleMath::Vector2 GetSize() const;


private:
    DirectX::SimpleMath::Vector2 position_;
    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;
    ID3D11InputLayout* layout_;
    DirectX::XMFLOAT4 points_[8];
    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;
    ID3D11Buffer* constBuffer_;
    ID3D11RasterizerState* rastState_;
    ID3DBlob* vertexShaderByteCode_;
    ID3DBlob* pixelShaderByteCode_;
    UINT strides_[1];
    UINT offsets_[1];
    DirectX::SimpleMath::Vector4 offset;
};