#include "RacketComponent.h"
#include "Game.h"
#include <d3dcompiler.h>
#include <iostream>

RacketComponent::RacketComponent(Game* g) : GameComponent(g), mat(DirectX::XMMatrixIdentity()) {
    DirectX::XMFLOAT4 pointsTmp[8] = {
        DirectX::XMFLOAT4(0.03f, 0.2f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        DirectX::XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        DirectX::XMFLOAT4(0.03f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        DirectX::XMFLOAT4(0.0f, 0.2f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
    };
    std::swap(points_, pointsTmp);
}

RacketComponent::~RacketComponent() {
    DestroyResources();
}

void RacketComponent::Initialize() {
	ID3DBlob* errorVertexCode = nullptr;
	auto res = D3DCompileFromFile(L"..\\VertexShader.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
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
			// MessageBox(game->Display->hWnd, L"VertexShader.hlsl", L"Missing Shader File", MB_OK);
		}

		return;
	}

	D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	ID3DBlob* errorPixelCode;
	res = D3DCompileFromFile(L"..\\VertexShader.hlsl",
		Shader_Macros /*macros*/,
		nullptr /*include*/,
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
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(DirectX::SimpleMath::Vector4) * std::size(points_);

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = points_;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	game->Device->CreateBuffer(&vertexBufDesc, &vertexData, &vb_);

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

	game->Device->CreateBuffer(&indexBufDesc, &indexData, &ib_);

	strides_[0] = 32;
	offsets_[0] = 0;

	HRESULT hr = this->constantBuffer.Initialize(game->Device.Get(), game->Context);
	if (FAILED(hr))
	{
		return;
	}

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = game->Device->CreateRasterizerState(&rastDesc, &rastState_);
}

void RacketComponent::Update() {
	game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &constantBuffer.data, 0, 0);
}

void RacketComponent::UpdateRotation() {
	DirectX::XMMATRIX rotationMatrix = mat;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	game->Context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &rotationMatrix, sizeof(DirectX::XMMATRIX));
	game->Context->Unmap(constantBuffer.Get(), 0);
}

void RacketComponent::Draw() {
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

	constantBuffer.data.world = XMMatrixTranspose(mat);
	constantBuffer.data.view = DirectX::XMMatrixIdentity();
	constantBuffer.data.projection = DirectX::XMMatrixIdentity();
	if (!constantBuffer.ApplyChanges())
	{
		return;
	}
	game->Context->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());

	game->Context->DrawIndexed(6, 0, 0);
}

void RacketComponent::Reload() {
	DestroyResources();
	Initialize();
}

void RacketComponent::DestroyResources() {
	layout_->Release();
	pixelShader_->Release();
	pixelShaderByteCode_->Release();
	rastState_->Release();
	vertexShader_->Release();
	vertexShaderByteCode_->Release();
	vb_->Release();
	ib_->Release();
}

void RacketComponent::SetPosition(float x, float y) {
	mat.r[3] = DirectX::XMVectorSet(x, y, 0.0f, 1.0f);
}

void RacketComponent::MoveUp(float amount) {
	SetPosition(GetPosition().x, GetPosition().y + amount);
}

void RacketComponent::MoveDown(float amount) {
	SetPosition(GetPosition().x, GetPosition().y - amount);
}

void RacketComponent::MoveLeft(float amount) {
	SetPosition(GetPosition().x - amount, GetPosition().y);
}

void RacketComponent::MoveRight(float amount) {
	SetPosition(GetPosition().x + amount, GetPosition().y);
}

void RacketComponent::Rotate(float angle) {
	rotationAngle += angle;
	DirectX::XMFLOAT3 pos = GetPosition();
	DirectX::XMMATRIX translationMat = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	DirectX::XMMATRIX rotationMat = DirectX::XMMatrixRotationZ(rotationAngle);
	DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	mat = scaleMat * rotationMat * translationMat;
}

void RacketComponent::Rotate(float angle, float x, float y) {
	DirectX::XMFLOAT3 pos = GetPosition();
	float localX = -(x - pos.x);
	float localY = -(y - pos.y);

	DirectX::XMMATRIX translationToPivot = DirectX::XMMatrixTranslation(-localX, -localY, 0.0f);
	DirectX::XMMATRIX rotationMat = DirectX::XMMatrixRotationZ(angle);
	DirectX::XMMATRIX translationBack = DirectX::XMMatrixTranslation(localX, localY, 0.0f);

	mat = translationBack * rotationMat * translationToPivot * mat;
}

DirectX::XMFLOAT3 RacketComponent::GetPosition() const {
	return {mat.r[3].m128_f32[0], mat.r[3].m128_f32[1], mat.r[3].m128_f32[2]};
}

DirectX::SimpleMath::Vector2 RacketComponent::GetSize() const {
	return DirectX::SimpleMath::Vector2(0.03f, 0.2f);
}

DirectX::XMMATRIX RacketComponent::GetMatrix() const {
	return mat;
}
