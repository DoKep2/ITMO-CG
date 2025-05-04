#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <wrl/client.h>

struct Vertex2;

struct Material
{
    DirectX::XMFLOAT3 diffuseColor;
    float opacity;

    DirectX::XMFLOAT3 ambientColor;
    float hasTexture;

    DirectX::XMFLOAT3 specularColor;
    float pad;
};

class Mesh {
public:
    Mesh(Microsoft::WRL::ComPtr<ID3D11Device> device,  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, const std::vector<Vertex2>& vertices, const std::vector<uint32_t>& indices);
    Mesh(const Mesh& Mesh);

    void Draw() const;

    ID3D11Buffer* GetVertexBuffer() const { return vertexBuffer.Get(); }
    ID3D11Buffer* GetIndexBuffer() const { return indexBuffer.Get(); }
    UINT indicesSize = 0;
private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

};
