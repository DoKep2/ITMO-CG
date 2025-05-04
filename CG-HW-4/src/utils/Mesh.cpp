#include "Mesh.h"

#include <iostream>
#include <ostream>
#include <SphereComponent.h>

Mesh::Mesh(Microsoft::WRL::ComPtr<ID3D11Device> device,  Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
           const std::vector<Vertex2>& vertices, const std::vector<uint32_t>& indices)
{
    this->indicesSize = indices.size();
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(Vertex2) * vertices.size();
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices.data();
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateBuffer(&bufferDesc, &vertexData, vertexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        // Handle error
    }
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(uint32_t) * indices.size();
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices.data();
    hr = device->CreateBuffer(&bufferDesc, &indexData, indexBuffer.GetAddressOf());
    if (FAILED(hr)) {
        // Handle error
    }
    UINT stride = sizeof(Vertex2);
    UINT offset = 0;
    this->deviceContext = context;
    this->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    this->deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    this->deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

Mesh::Mesh(const Mesh &Mesh) {
    this->deviceContext = Mesh.deviceContext;
    this->vertexBuffer = Mesh.vertexBuffer;
    this->indexBuffer = Mesh.indexBuffer;
    this->indicesSize = Mesh.indicesSize;
}

void Mesh::Draw() const {
    UINT offset = 0;
    UINT stride = sizeof(Vertex2);
    this->deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    this->deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    this->deviceContext->DrawIndexed(indicesSize, 0, 0); // Draw the mesh using the index buffer
}
