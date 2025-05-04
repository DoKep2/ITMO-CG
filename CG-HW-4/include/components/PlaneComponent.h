#pragma once

#include <ConstantBufferTypes.h>

#include "GameComponent.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include <DirectXMath.h>
#include <wrl/client.h>
#include <string>

class PlaneComponent : public GameComponent {
public:
	PlaneComponent(Game* g, std::wstring texturePath, float width, float depth);

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DestroyResources() override;
	void Reload() override;

	~PlaneComponent();

private:

	struct Vertex {
		XMFLOAT4 pos;
		XMFLOAT2 tex;
	};

	std::wstring texturePath;
	float width_;
	float depth_;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode_;
	// SimpleMath::Vector4 points_[4];
	Vertex vertices_[4]; // заменяет points_
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rastState_;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode_;
	ConstantBuffer<CB_VS_vertexshader> constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vb_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ib_;

	XMMATRIX mat = XMMatrixIdentity();

	UINT strides_[1];
	UINT offsets_[1];
	int indices_[6];
};
