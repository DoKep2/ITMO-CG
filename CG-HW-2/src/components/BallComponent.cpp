#include "BallComponent.h"
#include "Game.h"
#include <d3dcompiler.h>
#include <iostream>
#include "PongGame.h"
#include "DirectXCollision.h"

const float kBallAcceleration = 0.002f;

BallComponent::BallComponent(Game* g) : GameComponent(g), velocity_(0.0f, 0.0f) {
	DirectX::XMFLOAT4 points[100];
	for (int i = 0; i < 50; i++) {
		float angle = DirectX::XM_2PI * i / 50;
		points[i * 2] = DirectX::XMFLOAT4(std::cos(angle) * 0.025f, std::sin(angle) * 0.025f, 0.5f, 1.0f);
		points[i * 2 + 1] = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	std::swap(points_, points);
	/*DirectX::XMFLOAT4 pointsTmp[8] = {
		DirectX::XMFLOAT4(0.025f, 0.025f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0.025f, 0.0f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0.0f, 0.025f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	};
	std::swap(points_, pointsTmp);*/
}

BallComponent::~BallComponent() {
	DestroyResources();
}

void BallComponent::Reload()
{
	DestroyResources();
	Initialize();
}

void BallComponent::Initialize() {
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

	int indeces[300];
	for (int i = 0; i < 100; i++)
	{
		indeces[i * 3] = 0;
		indeces[i * 3 + 1] = (i + 1) % 50;
		indeces[i * 3 + 2] = (i + 2) % 50;
	}
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

	D3D11_BUFFER_DESC constBufDesc = {};
	constBufDesc.Usage = D3D11_USAGE_DEFAULT;
	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.CPUAccessFlags = 0;
	constBufDesc.MiscFlags = 0;
	constBufDesc.StructureByteStride = 0;
	constBufDesc.ByteWidth = sizeof(DirectX::SimpleMath::Vector4);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = game->Device->CreateRasterizerState(&rastDesc, &rastState_);

}

void BallComponent::Update() {
	SetPosition(GetPosition().x + velocity_.x, GetPosition().y + velocity_.y);
	if (IsOutOfBounds()) {
		static_cast<PongGame*>(game)->UpdateResult();
		SetPosition(0.0f, 0.0f);
		std::pair<float, float> dir = static_cast<PongGame*>(game)->GenerateStartBallDirection();
		SetVelocity(dir.first, dir.second);
		//SetVelocity(0.015, 0.015);
	}

	if (IsCollidedWithVerticalWall()) {
		velocity_.y = -velocity_.y;
	}


	for(auto& c : game->Components)
	{
		if (c != this)
		{
			auto racket = dynamic_cast<RacketComponent*>(c);
			if (racket)
			{
				CheckCollision(racket);
			}
		}
	}

	// game->Context->UpdateSubresource(constBuffer_, 0, 0, &position_, 0, 0);
	game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &constantBuffer.data, 0, 0);
}

void BallComponent::Draw() {
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

	game->Context->DrawIndexed(300, 0, 0);
}

void BallComponent::DestroyResources() {
	layout_->Release();
	pixelShader_->Release();
	pixelShaderByteCode_->Release();
	rastState_->Release();
	vertexShader_->Release();
	vertexShaderByteCode_->Release();
	vb_->Release();
	ib_->Release();
	// constantBuffer->Release();
}

void BallComponent::SetVelocity(float x, float y) {
	velocity_.x = x;
	velocity_.y = y;
}

void BallComponent::SetPosition(float x, float y) {
	DirectX::XMMATRIX translationMat = DirectX::XMMatrixTranslation(x, y, 0.0f);
	DirectX::XMMATRIX rotationMat = DirectX::XMMatrixRotationY(0.0f);
	DirectX::XMMATRIX scaleMat = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	mat = scaleMat * rotationMat * translationMat;
}

void BallComponent::CheckCollision(RacketComponent* racket) {
	DirectX::XMFLOAT3 racketPos = racket->GetPosition();
	DirectX::XMFLOAT3 pos = GetPosition();
	DirectX::SimpleMath::Vector2 racketSize = racket->GetSize();
	DirectX::BoundingOrientedBox racketBox;
	DirectX::BoundingSphere ballSphere;
	racketBox.Center = DirectX::XMFLOAT3(racketPos.x + racketSize.x / 2, racketPos.y + racketSize.y / 2, 0.0f);
	racketBox.Extents = DirectX::XMFLOAT3(racketSize.x / 2, racketSize.y / 2, 0.0f);
	DirectX::XMVECTOR quaternion = DirectX::XMQuaternionRotationMatrix(racket->GetMatrix());
	XMStoreFloat4(&racketBox.Orientation, quaternion);
	ballSphere.Center = DirectX::XMFLOAT3(pos.x, pos.y, 0.0f);
	ballSphere.Radius = 0.025f;

	if (racketBox.Intersects(ballSphere) && pos.x >= -0.95 + racket->GetSize().x && pos.x <= 0.92) {
		ChangeVelocityAfterCollision(racket);


		/*auto ball = new BallComponent(game);
		ball->Initialize();
		ball->SetPosition(position_.x, position_.y);
		ball->SetVelocity(velocity_.x, velocity_.y + 0.01);
		game->ComponentsToAdd.push_back(ball);*/
	}
}

bool BallComponent::IsOutOfBounds() const {
	auto pos = GetPosition();
	return pos.x > 1.0f || pos.x < -1.0f;
}

bool BallComponent::IsCollidedWithVerticalWall() const {
	auto pos = GetPosition();
	return pos.y > 1.0f || pos.y < -1.0f;
}

DirectX::XMFLOAT3 BallComponent::GetPosition() const {
	return {mat.r[3].m128_f32[0], mat.r[3].m128_f32[1], mat.r[3].m128_f32[2]};
}

void BallComponent::ChangeVelocityAfterCollision(RacketComponent* racket) {
	float racketHeight = racket->GetSize().y;
	float racketY = racket->GetPosition().y;
	float ballY = GetPosition().y;

	float relativeIntersectY = (ballY - racketY) - (racketHeight / 2.0f);
	float normalizedRelativeIntersectionY = relativeIntersectY / (racketHeight / 2.0f);

	if (normalizedRelativeIntersectionY > 1.0f) normalizedRelativeIntersectionY = 1.0f;
	if (normalizedRelativeIntersectionY < -1.0f) normalizedRelativeIntersectionY = -1.0f;

	float maxBounceAngle = DirectX::XM_PI / 3.0f;
	float bounceAngle = normalizedRelativeIntersectionY * maxBounceAngle;

	float speed = sqrt(velocity_.x * velocity_.x + velocity_.y * velocity_.y);
	velocity_.x = speed * cos(bounceAngle) * std::copysign(1.0f, -velocity_.x);
	velocity_.y = speed * sin(bounceAngle);

	velocity_.x += std::copysign(kBallAcceleration, velocity_.x);
	velocity_.y += std::copysign(kBallAcceleration, velocity_.y);
}

