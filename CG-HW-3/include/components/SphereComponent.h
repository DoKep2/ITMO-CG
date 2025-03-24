#pragma once

#include <string>

#include "Camera.h"
#include "Game.h"
#include "ConstantBuffer.h"
#include "ConstantBufferTypes.h"
#include <GameComponent.h>
#include <SimpleMath.h>

#include <wrl/client.h>

const int num = 50;

class SphereComponent : public GameComponent
{
private:
	ID3D11InputLayout* layout_;
	ID3D11PixelShader* pixelShader_;
	ID3DBlob* pixelShaderByteCode_;
	SimpleMath::Vector4 points_[num * (num + 1) + 1];
	ID3D11RasterizerState* rastState_;
	ID3D11VertexShader* vertexShader_;
	ID3DBlob* vertexShaderByteCode_;
	ConstantBuffer<CB_VS_vertexshader> constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	ID3D11ShaderResourceView* textureSRV;
	int indices_[num * num * 6];
	SimpleMath::Vector4 offset;
	SimpleMath::Matrix mat = SimpleMath::Matrix::Identity;
	float rotationAngle = 0.0f;
	SimpleMath::Vector2 velocity_;
	float orbitalVelocity;
	XMFLOAT3 position_ = { 0.0f, 0.0f, 0.0f };
	float orbitAngle_;
	SphereComponent* orbitCenter_;
	float orbitDistance_;
	std::wstring texturePath;
	static constexpr int stackCount_ = num;
	static constexpr int sliceCount_ = num;
	const float radius_ = 0.5;
	ID3D11Buffer* vb_;
	ID3D11Buffer* ib_;
	UINT strides_[1];
	UINT offsets_[1];

	void RotateByCenter(float angle);
public:
	SphereComponent(Game* g, std::wstring texturePath, float radius, float orbitalVelocity = 0.0f);
	Camera camera;
	virtual ~SphereComponent();
	void DestroyResources() override;
	void Draw() override;
	void Initialize() override;
	void Update() override;
	void Reload() override;
	void SetPosition(float x, float y, float z);
	void SetRotation(float angle);
	void RotateAround(SphereComponent* center, float distance, float angle);
	void BindCameraToSphere(SphereComponent* sphere);
};

