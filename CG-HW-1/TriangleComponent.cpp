//
// Created by sergo on 18.02.2025.
//

#include "TriangleComponent.h"
#include "Game.h"
#include <d3dcompiler.h>
#include <iostream>
#include <random>

using namespace DirectX;

TriangleComponent::TriangleComponent(Game* g) : GameComponent(g), mat(XMMatrixIdentity())
{
	SimpleMath::Vector4 pointsTmp[6] = {
		DirectX::XMFLOAT4(0.0f, 0.05f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(-0.05f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0.05f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	};
	std::swap(points_, pointsTmp);
}

TriangleComponent::~TriangleComponent()
{
}

void TriangleComponent::DestroyResources() {
	layout_->Release();
	pixelShader_->Release();
	pixelShaderByteCode_->Release();
	rastState_->Release();
	vertexShader_->Release();
	vertexShaderByteCode_->Release();
	vb_->Release();
	ib_->Release();
}


void TriangleComponent::Draw() {
	game->Context->RSSetState(rastState_);

	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(game->Display->ClientWidth);
	viewport.Height = static_cast<float>(game->Display->ClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;

	game->Context->RSSetViewports(1, &viewport);

	game->Context->IASetInputLayout(layout_);
	game->Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game->Context->IASetIndexBuffer(ib_, DXGI_FORMAT_R32_UINT, 0);
	game->Context->IASetVertexBuffers(0, 1, &vb_, strides_, offsets_);
	game->Context->VSSetShader(vertexShader_, nullptr, 0);
	game->Context->PSSetShader(pixelShader_, nullptr, 0);

	// constantBuffer.data.xOffset = offset.x;
	// constantBuffer.data.yOffset = offset.y;
	// constantBuffer.data.mat = mat;
	// constantBuffer.data.mat = DirectX::XMMatrixTranspose(constantBuffer.data.mat);
	constantBuffer.data.world = XMMatrixTranspose(mat);
	constantBuffer.data.view = XMMatrixIdentity();
	constantBuffer.data.projection = XMMatrixIdentity();

	if (!constantBuffer.ApplyChanges())
	{
		return;
	}
	game->Context->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());

	game->Context->DrawIndexed(6, 0, 0);
}

void TriangleComponent::Initialize() {
	ID3DBlob* errorVertexCode = nullptr;

	auto res = D3DCompileFromFile(L"..\\aboba.hlsl",
		nullptr,
		nullptr,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexShaderByteCode_,
		&errorVertexCode);

	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorVertexCode) {
			char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(game->Display->hWnd, reinterpret_cast<LPCSTR>(L"MyVeryFirstShader.hlsl"), reinterpret_cast<LPCSTR>(L"Missing Shader File"), MB_OK);
		}

		return;
	}

	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(L"..\\aboba.hlsl",
		Shader_Macros,
		nullptr,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelShaderByteCode_,
		&errorPixelCode);

	game->Device->CreateVertexShader(
		vertexShaderByteCode_->GetBufferPointer(),
		vertexShaderByteCode_->GetBufferSize(),
		nullptr, &vertexShader_);

	game->Device->CreatePixelShader(
		pixelShaderByteCode_->GetBufferPointer(),
		pixelShaderByteCode_->GetBufferSize(),
		nullptr, &pixelShader_);

	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	game->Device->CreateInputLayout(
		inputElements,
		2,
		vertexShaderByteCode_->GetBufferPointer(),
		vertexShaderByteCode_->GetBufferSize(),
		&layout_);

	D3D11_BUFFER_DESC vertexBufDesc = {};
	//vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * std::size(points_);

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = points_;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;


	game->Device->CreateBuffer(&vertexBufDesc, &vertexData, &vb_);
	int indeces[] = { 0,1,2 };
	//int indeces[] = { 0,1,2, 0,1,3 };
	D3D11_BUFFER_DESC indexBufDesc = {};
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = sizeof(int) * std::size(indeces);

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indeces;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	game->Device->CreateBuffer(&indexBufDesc, &indexData, &ib_);

	strides_[0] = 32;
	offsets_[0] = 0;

	HRESULT hr = this->constantBuffer.Initialize(game->Device.Get(), game->Context);
	if (FAILED(hr))
	{
		return;
	}

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = game->Device->CreateRasterizerState(&rastDesc, &rastState_);
}

void TriangleComponent::Update()
{
	SetPosition(GetPosition().x + velocity_.x, GetPosition().y + velocity_.y);
	// game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &offset, 0, 0);
	game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &constantBuffer.data, 0, 0);
	for(auto& c : game->Components)
	{
		if (c != this)
		{
			auto square = dynamic_cast<SquareComponent*>(c);
			if (square)
			{
				CheckCollision(square);
			}
		}
	}
	// std::cout << offset.x << ' ' << offset.y << std::endl;
}

void TriangleComponent::Reload()
{
}

void TriangleComponent::SetPosition(float x, float y) {
	mat.r[3] = DirectX::XMVectorSet(x, y, 0.0f, 1.0f);
}

XMFLOAT3 TriangleComponent::GetPosition() const {
	return {mat.r[3].m128_f32[0], mat.r[3].m128_f32[1], mat.r[3].m128_f32[2]};
};

void TriangleComponent::SetRotation(float angle, float x, float y)
{
	DirectX::XMFLOAT3 pos = GetPosition();
	float localX = -(x - pos.x);
	float localY = -(y - pos.y);

	DirectX::XMMATRIX translationToPivot = DirectX::XMMatrixTranslation(-localX, -localY, 0.0f);
	DirectX::XMMATRIX rotationMat = DirectX::XMMatrixRotationZ(angle);
	DirectX::XMMATRIX translationBack = DirectX::XMMatrixTranslation(localX, localY, 0.0f);

	mat = translationBack * rotationMat * translationToPivot * mat;
}

void TriangleComponent::SetVelocity(float x, float y) {
	velocity_.x = x;
	velocity_.y = y;
}

void TriangleComponent::CheckCollision(SquareComponent *square) {
	XMFLOAT3 squarePos = square->GetPosition();
	SimpleMath::Vector2 squareSize = square->GetSize();
	BoundingBox squareBox;
	BoundingBox triangleBox;
	squareBox.Center = XMFLOAT3(squarePos.x, squarePos.y, 0.0f);
	squareBox.Extents = XMFLOAT3(squareSize.x / 2, squareSize.y / 2, 0.0f);
	triangleBox.Center = XMFLOAT3(GetPosition().x, GetPosition().y, 0.0f);
	triangleBox.Extents = XMFLOAT3(0.05f, 0.05f, 0.0f);
	if (triangleBox.Intersects(squareBox)) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(0.0f, 0.8f);
		square->SetPosition(dis(gen), dis(gen));
		SetPosition(10.0f, 10.0f);
	}
}

DirectX::SimpleMath::Vector4* TriangleComponent::GetPoints() {
	auto tmp = points_;
	auto res = new DirectX::SimpleMath::Vector4[3];
	for (int i = 0; i < 3; i++) {
		res[i] = tmp[i * 2];
	}
	return res;
}

