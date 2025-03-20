#pragma once
#include "GameComponent.h"
#include "RacketComponent.h"
#include "SimpleMath.h"

class BallComponent : public GameComponent {
public:
    BallComponent(Game* g);
    ~BallComponent();

    void Reload() override;
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void DestroyResources() override;

    void SetVelocity(float x, float y);
    void SetPosition(float x, float y);
    void CheckCollision(RacketComponent* racket);

	bool IsOutOfBounds() const;
	bool IsCollidedWithVerticalWall() const;
	void ChangeVelocityAfterCollision(RacketComponent* racket);

	DirectX::SimpleMath::Vector2 GetPosition() const;

private:
    DirectX::SimpleMath::Vector2 velocity_;
    DirectX::SimpleMath::Vector2 position_;
    DirectX::XMFLOAT4 points_[100];
    ID3D11Buffer* vb_;
    ID3D11Buffer* ib_;
    ID3D11InputLayout* layout_;
    ID3D11VertexShader* vertexShader_;
    ID3D11PixelShader* pixelShader_;
    ID3D11Buffer* constBuffer_;
    ID3D11RasterizerState* rastState_;
    ID3DBlob* vertexShaderByteCode_;
    ID3DBlob* pixelShaderByteCode_;
    UINT strides_[1];
    UINT offsets_[1];
};
