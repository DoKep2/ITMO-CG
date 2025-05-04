#pragma once

#include <string>

#include "Game.h"
#include "ConstantBuffer.h"
#include "ConstantBufferTypes.h"
#include <GameComponent.h>
#include <assimp/scene.h>

#include <wrl/client.h>

#include "Mesh.h"

class Aboba;
class Mesh;
const int num = 50;

struct Vertex2
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 texCoord;
};

class SphereComponent : public GameComponent
{
private:
	ID3D11InputLayout* layout_;
	ID3D11PixelShader* pixelShader_;
	ID3DBlob* pixelShaderByteCode_;
	std::vector<Vertex2> points_;
	ID3D11RasterizerState* rastState_;
	ID3D11VertexShader* vertexShader_;
	ID3DBlob* vertexShaderByteCode_;
	ConstantBuffer<CB_VS_vertexshader> constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	ConstantBuffer<Material> material;
	ID3D11ShaderResourceView* textureSRV;

	UINT strides_[1];
	UINT offsets_[1];
	std::vector<int> indices_;
	std::vector<Mesh> meshes_;
	std::vector<XMFLOAT3> vertices_;
	std::vector<ConstantBuffer<Material>> materials_;   // Для хранения материалов каждого меша
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textures_;   // Для хранения текстур (если есть)


	XMMATRIX mat = XMMatrixIdentity();

	float moveSpeed = 0.05f;
	float rotationSpeed = XMConvertToRadians(90.0f); // 90°/сек
	float totalRoll = 0.0f;
	float heading = 0.0f; // текущий угол поворота в радианах
	XMVECTOR velocity = XMVectorZero();   // текущая скорость шара
	float acceleration = 0.001f;   // меньше ускорение
	float friction = 0.98f;       // ближе к 1.0 — плавнее торможение

	std::wstring texturePath;

	void RotateByCenter(float angle);
public:
	SphereComponent(Game* g);

	virtual ~SphereComponent();
	void DestroyResources() override;
	void Draw() override;
	void Initialize() override;
	void Update() override;

	void CheckCollision(Aboba* aboba) const;

	void Reload() override;
	void SetPosition(const XMFLOAT3& newPosition);
	void SetRotation(const XMFLOAT4& newRotationQuat);
	XMVECTOR GetPosition() const;
	XMVECTOR GetRotation() const;

	void UpdateWorldMatrix();
	void HandleInput();
	void SetWorldMatrix(const XMMATRIX& world);
	XMMATRIX GetWorldMatrix() const;

	bool LoadModel(const std::string& modelPath);
	void ProcessNode(aiNode* node, const aiScene* scene, const std::string& modelPath);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
};

