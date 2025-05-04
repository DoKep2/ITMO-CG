#include "GridComponent.h"

#include <Camera.h>
#include <d3dcompiler.h>
#include <SphereComponent.h>

GridComponent::GridComponent(Game* game, int halfSize, float spacing)
    : GameComponent(game), halfSize_(halfSize), spacing_(spacing) {

    int lineCount = halfSize_ * 2 + 1;
    pointsCount_ = lineCount * 4; // по 2 вершины на каждую линию (X и Z)

    vertices_ = new GridVertex[pointsCount_];
    int index = 0;

    for (int i = -halfSize_; i <= halfSize_; ++i) {
        float coord = i * spacing_;

        // Линии вдоль X (по Z)
        vertices_[index++] = { XMFLOAT4(-halfSize_ * spacing_, 0, coord, 1.0f), GetColor(i == 0, true) };
        vertices_[index++] = { XMFLOAT4( halfSize_ * spacing_, 0, coord, 1.0f), GetColor(i == 0, true) };

        // Линии вдоль Z (по X)
        vertices_[index++] = { XMFLOAT4(coord, 0, -halfSize_ * spacing_, 1.0f), GetColor(i == 0, false) };
        vertices_[index++] = { XMFLOAT4(coord, 0,  halfSize_ * spacing_, 1.0f), GetColor(i == 0, false) };
    }
}

GridComponent::~GridComponent() {
    DestroyResources();
    delete[] vertices_;
}

XMFLOAT4 GridComponent::GetColor(bool isAxis, bool isZAxis) {
    if (isAxis)
        return isZAxis ? XMFLOAT4(0.0f, 0.4f, 1.0f, 0.3f) : XMFLOAT4(1.0f, 0.2f, 0.2f, 0.3f);
    return XMFLOAT4(0.5f, 0.5f, 0.5f, 0.1f); // обычные линии: серые и почти прозрачные
}


void GridComponent::Initialize() {
    ID3DBlob* vsCode;
    ID3DBlob* error;

    D3DCompileFromFile(L"..\\GridPixelShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsCode, &error);
    game->Device->CreateVertexShader(vsCode->GetBufferPointer(), vsCode->GetBufferSize(), nullptr, &vertexShader_);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    game->Device->CreateInputLayout(layoutDesc, 2, vsCode->GetBufferPointer(), vsCode->GetBufferSize(), &layout_);

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.ByteWidth = sizeof(GridVertex) * pointsCount_;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices_;

    game->Device->CreateBuffer(&vbDesc, &vbData, &vb_);

    ID3DBlob* psCode;
    D3DCompileFromFile(L"..\\GridPixelShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psCode, &error);
    game->Device->CreatePixelShader(psCode->GetBufferPointer(), psCode->GetBufferSize(), nullptr, &pixelShader_);

    HRESULT hr = this->constantBuffer.Initialize(game->Device.Get(), game->Context);
    if (FAILED(hr)) {
        return;
    }

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* blendState;
    game->Device->CreateBlendState(&blendDesc, &blendState);

    // Применение при отрисовке:
    float blendFactor[4] = { 0, 0, 0, 0 };
    game->Context->OMSetBlendState(blendState, blendFactor, 0xffffffff);
}

void GridComponent::Draw() {
    XMMATRIX identity = XMMatrixIdentity();

    static SphereComponent* sphere = static_cast<SphereComponent*> (game->Components[0]);

    Camera camera = sphere->camera;

    constantBuffer.data.world = XMMatrixTranspose(identity);
    constantBuffer.data.view = XMMatrixTranspose(camera.GetViewMatrix());
    constantBuffer.data.projection = XMMatrixTranspose(camera.GetProjectionMatrix());

    if (!constantBuffer.ApplyChanges()) {
        return;
    }

    game->Context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

    UINT stride = sizeof(GridVertex);
    UINT offset = 0;

    game->Context->IASetInputLayout(layout_);
    game->Context->IASetVertexBuffers(0, 1, &vb_, &stride, &offset);
    game->Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    game->Context->VSSetShader(vertexShader_, nullptr, 0);
    game->Context->PSSetShader(pixelShader_, nullptr, 0);

    game->Context->Draw(pointsCount_, 0);
}

void GridComponent::DestroyResources() {
    if (vb_) vb_->Release();
    if (layout_) layout_->Release();
    if (vertexShader_) vertexShader_->Release();
    if (pixelShader_) pixelShader_->Release();
}

void GridComponent::Update() {
    game->Context->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &constantBuffer.data, 0, 0);
}

void GridComponent::Reload() {
}
