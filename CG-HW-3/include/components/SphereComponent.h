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
	ID3D11Buffer* vb_;
	ID3D11Buffer* ib_;

	UINT strides_[1];
	UINT offsets_[1];
	int indices_[num * num * 6];

	XMMATRIX mat = XMMatrixIdentity();
	SphereComponent* orbitingTarget = nullptr;
	float orbitRadius = 0.0f;
	float orbitSpeed = 0.0f;
	float orbitAngle = 0.0f;
	float selfRotationAngle = 0.0f;
	float selfRotationSpeed = 1.0f;
	XMVECTOR orbitOffset = XMVectorZero();
	bool orbitOffsetInitialized = false;

	std::wstring texturePath;

	static constexpr int stackCount_ = num;
	static constexpr int sliceCount_ = num;
	const float radius_ = 0.5;

	void RotateByCenter(float angle);
public:
	SphereComponent(Game* g, std::wstring texturePath, float radius);
	Camera camera;
	virtual ~SphereComponent();
	void DestroyResources() override;
	void Draw() override;
	void Initialize() override;
	void Update() override;
	void Reload() override;
	void SetPosition(const XMFLOAT3& newPosition);
	void SetRotation(const XMFLOAT4& newRotationQuat);
	XMVECTOR GetPosition() const;
	XMVECTOR GetRotation() const;

	void UpdateWorldMatrix();
	void BindCameraToSphere(SphereComponent* sphere);
	void FollowCamera(SphereComponent* sphereToFollow) const;
	void HandleCameraInput();
	void SetOrbitingTarget(SphereComponent* target, float orbitRadius, float orbitSpeed);
	void SetWorldMatrix(const XMMATRIX& world);
	XMMATRIX GetWorldMatrix() const;
	void UpdateOrbit(float deltaTime);
};

