#pragma once
#include <ConstantBuffer.h>

#include "GameComponent.h"
#include "Game.h"
#include <DirectXMath.h>
#include <SphereComponent.h>

struct CB_VS_vertexshader;

class OrbitComponent : public GameComponent {
public:
	OrbitComponent(Game* game, float radius, int segmentCount = 128);
	virtual ~OrbitComponent();
	void DestroyResources() override;
	void Initialize() override;
	void Draw() override;
	void Update() override;
	void Reload() override;

	void SetCenter(SphereComponent* center);

private:
	float radius_;
	int segmentCount_;
	XMFLOAT4* points_;
	SphereComponent* centerObject_ = nullptr;

	ID3D11Buffer* vb_ = nullptr;
	ID3D11VertexShader* vertexShader_ = nullptr;
	ID3D11PixelShader* pixelShader_ = nullptr;
	ID3D11InputLayout* layout_ = nullptr;

	ConstantBuffer<CB_VS_vertexshader> constantBuffer;
};
