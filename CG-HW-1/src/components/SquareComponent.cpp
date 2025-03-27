#include "SquareComponent.h"
#include "Game.h"
#include <iostream>
#include "ConstantBufferTypes.h"

using namespace DirectX;

static float rotation = 0.0f;

SquareComponent::SquareComponent(Game* g) : GameComponent(g), mat(XMMatrixIdentity())
{
	SimpleMath::Vector4 pointsTmp[8] = {
	DirectX::XMFLOAT4(0.15f, 0.15f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),
	DirectX::XMFLOAT4(0.15f, 0.0f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),
	DirectX::XMFLOAT4(0.0f, 0.15f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),
	};
	std::swap(points, pointsTmp);
}

SquareComponent::~SquareComponent()
{
	DestroyResources();
}

void SquareComponent::DestroyResources()
{
	layout->Release();
	pixelShader->Release();
	pixelShaderByteCode->Release();
	rastState->Release();
	vertexShader->Release();
	vertexShaderByteCode->Release();
	vertexBuffer->Release();
	indexBuffer->Release();
	//constantBuffer->Release();
}

void SquareComponent::Draw()
{
	game->Context->RSSetState(rastState);

	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(game->Display->ClientWidth);
	viewport.Height = static_cast<float>(game->Display->ClientHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;

	game->Context->RSSetViewports(1, &viewport);

	game->Context->IASetInputLayout(layout);
	game->Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game->Context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	game->Context->IASetVertexBuffers(0, 1, &vertexBuffer, strides, offsets);
	game->Context->VSSetShader(vertexShader, nullptr, 0);
	game->Context->PSSetShader(pixelShader, nullptr, 0);


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

void SquareComponent::Initialize()
{
	ID3DBlob* errorVertexCode = nullptr;
	auto res = D3DCompileFromFile(L"..\\aboba.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexShaderByteCode,
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
			MessageBox(game->Display->hWnd, L"QuadShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return;
	}

	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(L"..\\aboba.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelShaderByteCode,
		&errorPixelCode);

	game->Device->CreateVertexShader(
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		nullptr, &vertexShader);

	game->Device->CreatePixelShader(
		pixelShaderByteCode->GetBufferPointer(),
		pixelShaderByteCode->GetBufferSize(),
		nullptr, &pixelShader);

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
		vertexShaderByteCode->GetBufferPointer(),
		vertexShaderByteCode->GetBufferSize(),
		&layout);

	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(SimpleMath::Vector4) * std::size(points);

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = points;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	game->Device->CreateBuffer(&vertexBufDesc, &vertexData, &vertexBuffer);

	int indeces[] = { 0,1,2, 1,0,3 };
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

	game->Device->CreateBuffer(&indexBufDesc, &indexData, &indexBuffer);

	strides[0] = 32;
	offsets[0] = 0;

	HRESULT hr = this->constantBuffer.Initialize(game->Device.Get(), game->Context);
	if (FAILED(hr))
	{
		return;
	}

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = game->Device->CreateRasterizerState(&rastDesc, &rastState);
}

void SquareComponent::Update()
{
	game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &constantBuffer.data, 0, 0);
}

void SquareComponent::Reload()
{
	DestroyResources();
	Initialize();
}

void SquareComponent::SetPosition(float x, float y)
{
	mat.r[3] = DirectX::XMVectorSet(x, y, 0.0f, 1.0f);
}

void SquareComponent::SetRotation(float angle)
{
	rotation += angle;
	XMFLOAT3 pos = GetPosition();
	XMMATRIX translationMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX rotationMat = XMMatrixRotationY(rotation);
	XMMATRIX scaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	mat = scaleMat * rotationMat * translationMat;

}

XMFLOAT3 SquareComponent::GetPosition() const
{
	return {mat.r[3].m128_f32[0], mat.r[3].m128_f32[1], mat.r[3].m128_f32[2]};
}

float SquareComponent::GetRotation() {
	return rotation;
}

DirectX::SimpleMath::Vector2 SquareComponent::GetSize() const
{
	return DirectX::SimpleMath::Vector2(0.15f, 0.15f);
}

DirectX::SimpleMath::Vector4* SquareComponent::GetPoints() const
{
	auto tmp = points;
	auto res = new DirectX::SimpleMath::Vector4[4];
	for (int i = 0; i < 4; i++) {
		res[i] = tmp[i * 2];
	}
	return res;
}
