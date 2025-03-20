#pragma once
#include "GameComponent.h"
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "SimpleMath.h"

class RectangleComponent :
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
	ID3D11Buffer* constBuffer;
	UINT strides[1];
	UINT offsets[1];
	DirectX::SimpleMath::Vector4 offset;
public:
	RectangleComponent(Game* g);
	virtual ~RectangleComponent();
	void DestroyResources() override;
	void Draw() override;
	void Initialize() override;
	void Update() override;
	void Reload() override;
	void SetPosition(float x, float y);
	void SetPosition(DirectX::SimpleMath::Vector2 pos);
	void SetY(float y);
	void SetX(float x);
	float GetX() const;
	float GetY() const;
	DirectX::SimpleMath::Vector2 GetPosition() const;
};

