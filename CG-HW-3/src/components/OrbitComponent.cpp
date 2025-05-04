#include "OrbitComponent.h"
#include "SphereComponent.h"
#include <d3dcompiler.h>

OrbitComponent::OrbitComponent(Game* game, float radius, int segmentCount)
    : GameComponent(game), radius_(radius), segmentCount_(segmentCount) {
    points_ = new XMFLOAT4[segmentCount_ + 1];
    float step = XM_2PI / segmentCount_;
    for (int i = 0; i <= segmentCount_; ++i) {
        float angle = step * i;
        points_[i] = XMFLOAT4(
            radius_ * cosf(angle),
            0.0f,
            radius_ * sinf(angle),
            1.0f
        );
    }
}

OrbitComponent::~OrbitComponent() {
    DestroyResources();
}

void OrbitComponent::DestroyResources() {
    layout_->Release();
    pixelShader_->Release();
    vertexShader_->Release();
    vb_->Release();
}

void OrbitComponent::Reload() {
}

void OrbitComponent::SetCenter(SphereComponent* center) {
    centerObject_ = center;
}

void OrbitComponent::Initialize() {
    ID3DBlob* vsCode;
    ID3DBlob* error;

    D3DCompileFromFile(L"..\\OrbitPixelShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vsCode, &error);

    game->Device->CreateVertexShader(vsCode->GetBufferPointer(), vsCode->GetBufferSize(), nullptr, &vertexShader_);

    D3D11_INPUT_ELEMENT_DESC inputDesc = {
        "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
        0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0
    };

    game->Device->CreateInputLayout(&inputDesc, 1, vsCode->GetBufferPointer(), vsCode->GetBufferSize(), &layout_);

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.ByteWidth = sizeof(XMFLOAT4) * (segmentCount_ + 1);

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = points_;

    game->Device->CreateBuffer(&vbDesc, &vbData, &vb_);

    ID3DBlob* psCode;
    D3DCompileFromFile(L"..\\OrbitPixelShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psCode, &error);
    game->Device->CreatePixelShader(psCode->GetBufferPointer(), psCode->GetBufferSize(), nullptr, &pixelShader_);

    HRESULT hr = this->constantBuffer.Initialize(game->Device.Get(), game->Context);
    if (FAILED(hr)) {
        return;
    }
}

void OrbitComponent::Update() {
    game->Context->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &constantBuffer.data, 0, 0);
}

void OrbitComponent::Draw() {
    XMMATRIX translation = XMMatrixIdentity();
    if (centerObject_) {
        translation = XMMatrixTranslationFromVector(centerObject_->GetPosition());
    }

    game->Context->IASetInputLayout(layout_);
    game->Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
    UINT stride = sizeof(XMFLOAT4);
    UINT offset = 0;
    game->Context->IASetVertexBuffers(0, 1, &vb_, &stride, &offset);

    game->Context->VSSetShader(vertexShader_, nullptr, 0);
    game->Context->PSSetShader(pixelShader_, nullptr, 0);

    static SphereComponent* sphere = static_cast<SphereComponent*> (game->Components[0]);

    Camera camera = sphere->camera;

    constantBuffer.data.world = XMMatrixTranspose(translation);
    constantBuffer.data.view = XMMatrixTranspose(camera.GetViewMatrix());
    constantBuffer.data.projection = XMMatrixTranspose(camera.GetProjectionMatrix());

    if (!constantBuffer.ApplyChanges()) {
        return;
    }

    game->Context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
    game->Context->Draw(segmentCount_ + 1, 0);
}
