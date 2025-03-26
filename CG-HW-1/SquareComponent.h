#pragma once
#include "GameComponent.h"
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "SimpleMath.h"
#include "ConstantBuffer.h"

class SquareComponent :
	public GameComponent
{
protected:
	ID3D11InputLayout* layout;
	ID3D11PixelShader* pixelShader;
	ID3DBlob* pixelShaderByteCode;
	DirectX::SimpleMath::Vector4 points[8];
	ID3D11RasterizerState* rastState;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* vertexShaderByteCode;
	int indices[6];
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	ConstantBuffer<CB_VS_vertexshader> constantBuffer;
	UINT strides[1];
	UINT offsets[1];
	DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();
public:
	SquareComponent(Game* g);
	virtual ~SquareComponent();
	void DestroyResources() override;
	void Draw() override;
	void Initialize() override;
	void Update() override;
	void Reload() override;
	void SetPosition(float x, float y);
	void SetRotation(float angle);
	float GetRotation();

	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::SimpleMath::Vector2 GetSize() const;
	DirectX::SimpleMath::Vector4* GetPoints() const;
};

