//
// Created by sergo on 18.02.2025.
//

#include "TriangleComponent.h"
#include "Game.h"
#include <d3dcompiler.h>
#include <iostream>

using namespace DirectX;

TriangleComponent::TriangleComponent(Game* g) : GameComponent(g), offset(), mat(XMMatrixIdentity())
{
	/*Vertex v[] = {
		Vertex(-0.5f, -0.5f, 1.0f, 0.0f, 1.0f),
		Vertex(0.0f, 0.5f, 1.0f, 0.5f, 0.0f),
		Vertex(0.5f, -0.5f, 1.0f, 1.0f, 1.0f)
	};*/
	DirectX::XMFLOAT4 pointsTmp[6] = {
		DirectX::XMFLOAT4(0.0f, 0.5f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(-0.5f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0.5f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	};

	//std::swap(points_, v);
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
	//game->Context->PSSetSamplers(0, 1, samplerState.GetAddressOf());

	constantBuffer.data.xOffset = offset.x;
	constantBuffer.data.yOffset = offset.y;
	constantBuffer.data.mat = mat;
	constantBuffer.data.mat = DirectX::XMMatrixTranspose(constantBuffer.data.mat);
	if (!constantBuffer.ApplyChanges())
	{
		return;
	}
	game->Context->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());

	game->Context->DrawIndexed(6, 0, 0);
}

void TriangleComponent::Initialize() {
	ID3DBlob* errorVertexCode = nullptr;

	//auto res = D3DCompileFromFile(L"C:\\Users\\sergo\\Downloads\\TextureShader.hlsl",
	auto res = D3DCompileFromFile(L"C:\\Users\\sergo\\Downloads\\MyVeryFirstShader.hlsl",
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
			// MessageBox(game->Display->hWnd, L"MyVeryFirstShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return;
	}

	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };
    //D3D_SHADER_MACRO Shader_Macros[] = {{ "TEXCOORD", "float2" }, { nullptr, nullptr }};
	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(L"C:\\Users\\sergo\\Downloads\\MyVeryFirstShader.hlsl",
	//res = D3DCompileFromFile(L"C:\\Users\\sergo\\Downloads\\TextureShader.hlsl",
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
	//int indeces[] = { 0,1,2 };
	int indeces[] = { 0,1,2, 0,1,3 };
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

	//D3D11_SAMPLER_DESC sampDesc;
	//ZeroMemory(&sampDesc, sizeof(sampDesc));
	//sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	//sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	//sampDesc.MinLOD = 0;
	//sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	//hr = game->Device->CreateSamplerState(&sampDesc, samplerState.GetAddressOf());
	//if (FAILED(hr))
	//{
	//	return;
	//}
}

void TriangleComponent::Update()
{
	/*D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = game->Context->Map(vb_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (SUCCEEDED(hr))
	{
		memcpy(mappedResource.pData, points_, sizeof(points_));
		game->Context->Unmap(vb_, 0);
	}
		
	game->Context->UpdateSubresource(constBuffer_, 0, 0, &position_, 0, 0);*/
	SetPosition(position_.x + velocity_.x, position_.y + velocity_.y);
	game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &position_, 0, 0);
	game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &constantBuffer.data, 0, 0);
}

void TriangleComponent::Reload()
{
}

void TriangleComponent::SetPosition(float x, float y) {
	offset.x = x;
	offset.y = y;
	position_.x = x;
	position_.y = y;
}

DirectX::SimpleMath::Vector4 TriangleComponent::GetPosition() const {
	return offset;
};

void TriangleComponent::SetRotation(float angle)
{
	rotationAngle += angle;
	mat = XMMatrixRotationZ(rotationAngle);
}

void TriangleComponent::SetVelocity(float x, float y) {
	velocity_.x = x;
	velocity_.y = y;
}