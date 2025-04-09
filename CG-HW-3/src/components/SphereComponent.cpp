#include "SphereComponent.h"
#include "GameComponent.h"
#include "Game.h"
#include <d3dcompiler.h>
#include "DDSTextureLoader.h"
#include <iostream>

float SmoothStep(float t);
static float cameraSpeed = 0.01f;
static bool isFollowing = false;
static float interpolation = 0.0f;
static SphereComponent* followingSphere;

SphereComponent::SphereComponent(Game *g, std::wstring texturePath, float radius, float orbitalVelocity)
    : GameComponent(g), mat(XMMatrixIdentity()),
      texturePath(texturePath), radius_(radius), orbitalVelocity(orbitalVelocity)
{
    float phiStep = XM_PI / stackCount_;
    float thetaStep = 2.0f * XM_PI / sliceCount_;

    // Generate vertices
    for (int i = 0; i <= stackCount_; ++i) {
        float phi = i * phiStep;
        for (int j = 0; j <= sliceCount_; ++j) {
            float theta = j * thetaStep;
            float x = radius_ * sinf(phi) * cosf(theta);
            float y = radius_ * cosf(phi);
            float z = radius_ * sinf(phi) * sinf(theta);
            if(x == 0.0f && y == 0.0f && z == 0.0f) {
                continue;
            }
            points_[i * (sliceCount_ + 1) + j] = XMFLOAT4(x, y, z, 1.0f);
        }
    }

    // Generate indices
    int index = 0;
    for (int i = 0; i < stackCount_ - 26; ++i) {
        for (int j = 0; j < sliceCount_ - 25; ++j) {
            int first = i * (sliceCount_ + 1) + j;
            int second = first + sliceCount_ + 1;

            indices_[index++] = first;
            indices_[index++] = second;
            indices_[index++] = first + 1;

            indices_[index++] = second;
            indices_[index++] = second + 1;
            indices_[index++] = first + 1;
        }
    }
}

void SphereComponent::Initialize() {
    ID3DBlob *errorVertexCode = nullptr;

    auto res = D3DCompileFromFile(L"..\\TextureShader.hlsl",
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
            char *compileErrors = (char *) (errorVertexCode->GetBufferPointer());

            std::cout << compileErrors << std::endl;
        }
        // If there was  nothing in the error message then it simply could not find the shader file itself.
        else {
            MessageBox(game->Display->hWnd, L"TextureShader.hlsl", L"Missing Shader File", MB_OK);
        }

        return;
    }

    D3D_SHADER_MACRO Shader_Macros[] = {{"TEXCOORD", "float2"}, {nullptr, nullptr}};

    ID3DBlob *errorPixelCode;
    res = D3DCompileFromFile(L"..\\TextureShader.hlsl",
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
        D3D11_INPUT_ELEMENT_DESC{
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        D3D11_INPUT_ELEMENT_DESC{
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
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
    vertexBufDesc.ByteWidth = sizeof(SimpleMath::Vector4) * std::size(points_);

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = points_;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;


    game->Device->CreateBuffer(&vertexBufDesc, &vertexData, &vb_);
    D3D11_BUFFER_DESC indexBufDesc = {};
    indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufDesc.CPUAccessFlags = 0;
    indexBufDesc.MiscFlags = 0;
    indexBufDesc.StructureByteStride = 0;
    indexBufDesc.ByteWidth = sizeof(int) * std::size(indices_);

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices_;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    game->Device->CreateBuffer(&indexBufDesc, &indexData, &ib_);

    strides_[0] = 32;
    offsets_[0] = 0;

    HRESULT hr = this->constantBuffer.Initialize(game->Device.Get(), game->Context);
    if (FAILED(hr)) {
        return;
    }

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_FRONT;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    // rastDesc.FillMode = D3D11_FILL_WIREFRAME;

    res = game->Device->CreateRasterizerState(&rastDesc, &rastState_);

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = game->Device->CreateSamplerState(&sampDesc, samplerState.GetAddressOf());
    if (FAILED(hr)) {
        return;
    }

    hr = DirectX::CreateDDSTextureFromFile(
        game->Device.Get(),
        texturePath.c_str(),
        nullptr,
        &textureSRV
    );

    if (FAILED(hr)) {
        MessageBox(0, L"Texture not found", L"Error", MB_OK);
    }

    camera.SetPosition(0.0f, 0.0f, -2.0f);
    camera.SetProjectionValues(
        90.0f, static_cast<float>(game->Display->ClientWidth) / static_cast<float>(game->Display->ClientHeight), 0.1f,
        10000.0f);
}

void SphereComponent::DestroyResources() {
    layout_->Release();
    pixelShader_->Release();
    pixelShaderByteCode_->Release();
    rastState_->Release();
    vertexShader_->Release();
    vertexShaderByteCode_->Release();
    vb_->Release();
    ib_->Release();
    textureSRV->Release();
}

void SphereComponent::Draw() {
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
    game->Context->PSSetSamplers(0, 1, samplerState.GetAddressOf());
    game->Context->PSSetShaderResources(0, 1, &textureSRV);
    //fixmetodo
    // game->Context->OMSetDepthStencilState(game->depthStencilState, 0);

    constantBuffer.data.world = XMMatrixTranspose(mat);
    constantBuffer.data.view = XMMatrixTranspose(camera.GetViewMatrix());
    constantBuffer.data.projection = XMMatrixTranspose(camera.GetProjectionMatrix());

    if (!constantBuffer.ApplyChanges()) {
        return;
    }
    game->Context->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());
    game->Context->DrawIndexed(std::size(indices_), 0, 0);
}

void SphereComponent::Update() {
    auto pos = GetPosition();
    RotateAroundSphereY(orbitalVelocity, 0.0f, 0.0f, 0.0f, pos.z);

    HandleCameraInput();

    static XMVECTOR basePos;
    static XMVECTOR baseRot;

    if (isFollowing) {
        auto followingSpherePos = followingSphere->GetPosition();
        auto followingSphereRot = followingSphere->GetRotation();
        basePos = XMVectorSet(followingSpherePos.x, followingSpherePos.y, followingSpherePos.z - 0.5 - this->radius_, 0.0f);
        baseRot = XMVectorSet(followingSphereRot.x, followingSphereRot.y, followingSphereRot.z, 0.0f);
        auto currentPos = camera.GetPositionVector();
        auto currentRot = camera.GetRotationVector();

        interpolation += 0.001f;
        if (interpolation > 1.0f) {
            interpolation = 1.0f;
        }

        float smoothT = SmoothStep(interpolation);  // Применяем сглаживание
        auto newPos = XMVectorLerp(currentPos, basePos, smoothT);
        auto newRot = XMVectorLerp(currentRot, baseRot, smoothT);
        camera.SetPosition(newPos);
        camera.SetRotation(newRot);
    }
    game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &constantBuffer.data, 0, 0);
}

float SmoothStep(float t) {
    return t * t * (3.0f - 2.0f * t);
}


void SphereComponent::Reload() {
}

SphereComponent::~SphereComponent() {
    DestroyResources();
}

void SphereComponent::RotateByCenter(float angle) {
    rotationAngle += angle;
    XMFLOAT3 pos = GetPosition();
    XMMATRIX translationMat = XMMatrixTranslation(pos.x, pos.y, pos.z);
    XMMATRIX rotationMat = XMMatrixRotationY(rotationAngle);
    XMMATRIX scaleMat = XMMatrixScaling(1.0f, 1.0f, 1.0f);
    mat = scaleMat * rotationMat * translationMat;
}

void SphereComponent::RotateAroundY(float angle, float x, float y, float z) {
    DirectX::XMFLOAT3 pos = GetPosition();
    float localX = -(x - pos.x);
    float localY = -(y - pos.y);
    float localZ = -(z - pos.z);

    DirectX::XMMATRIX translationToPivot = DirectX::XMMatrixTranslation(-localX, -localY, -localZ);
    DirectX::XMMATRIX rotationMat = DirectX::XMMatrixRotationY(angle);
    DirectX::XMMATRIX translationBack = DirectX::XMMatrixTranslation(localX, localY, localZ);

    mat = translationBack * rotationMat * translationToPivot * mat;
}

void SphereComponent::RotateAroundSphereY(float angle, float sunX, float sunY, float sunZ, float orbitRadius) {
    DirectX::XMFLOAT3 pos = GetPosition();

    float localX = pos.x - sunX;
    float localY = pos.y - sunY;
    float localZ = pos.z - sunZ;

    DirectX::XMMATRIX translationToCenter = DirectX::XMMatrixTranslation(-sunX, -sunY, -sunZ);
    DirectX::XMMATRIX rotationMat = DirectX::XMMatrixRotationY(angle);
    DirectX::XMMATRIX translationBack = DirectX::XMMatrixTranslation(sunX, sunY, sunZ);

    // Применяем вращение
    DirectX::XMVECTOR positionVec = DirectX::XMVectorSet(localX, localY, localZ, 1.0f);
    positionVec = DirectX::XMVector3Transform(positionVec, rotationMat);

    DirectX::XMFLOAT3 newPos;
    DirectX::XMStoreFloat3(&newPos, positionVec);
    SetPosition(newPos.x + sunX, newPos.y + sunY, newPos.z + sunZ);

    mat = translationBack * rotationMat * translationToCenter * mat;
}

void SphereComponent::SetPosition(float x, float y, float z) {
    mat.r[3] = DirectX::XMVectorSet(x, y, z, 1.0f);
}

XMFLOAT3 SphereComponent::GetPosition() const {
    return {mat.r[3].m128_f32[0], mat.r[3].m128_f32[1], mat.r[3].m128_f32[2]};
}

XMFLOAT3 SphereComponent::GetRotation() const {
    return {rotationAngle, 0.0f, 0.0f};
}

void SphereComponent::HandleCameraInput() {
    if (game->InputDev->IsKeyDown(Keys::W)) {
        camera.AdjustPosition(camera.GetForwardVector() * cameraSpeed);
        isFollowing = false;
    }
    if (game->InputDev->IsKeyDown(Keys::A)) {
        camera.AdjustPosition(camera.GetLeftVector() * cameraSpeed);
        isFollowing = false;
    }

    if (game->InputDev->IsKeyDown(Keys::S)) {
        camera.AdjustPosition(camera.GetBackwardVector() * cameraSpeed);
        isFollowing = false;
    }

    if (game->InputDev->IsKeyDown(Keys::D)) {
        camera.AdjustPosition(camera.GetRightVector() * cameraSpeed);
        isFollowing = false;
    }

    if (game->InputDev->IsKeyDown(Keys::Space)) {
        camera.AdjustPosition(0.0f, cameraSpeed, 0.0f);
        isFollowing = false;
    }
    if (game->InputDev->IsKeyDown(Keys::LeftShift)) {
        camera.AdjustPosition(0.0f, -cameraSpeed, 0.0f);
        isFollowing = false;
    }

    if (game->InputDev->IsKeyDown(Keys::Up)) {
        camera.AdjustRotation(-cameraSpeed, 0.0f, 0.0f);
    }

    if (game->InputDev->IsKeyDown(Keys::Down)) {
        camera.AdjustRotation(cameraSpeed, 0.0f, 0.0f);
    }

    if (game->InputDev->IsKeyDown(Keys::Left)) {
        camera.AdjustRotation(0.0f, -cameraSpeed, 0.0f);
    }

    if (game->InputDev->IsKeyDown(Keys::Right)) {
        camera.AdjustRotation(0.0f, cameraSpeed, 0.0f);
    }

    if (game->InputDev->IsKeyDown(Keys::F1)) {
        auto* sphere = dynamic_cast<SphereComponent*>(game->Components[0]);
        followingSphere = sphere;
        isFollowing = true;
        interpolation = 0.0f;
    }

    if (game->InputDev->IsKeyDown(Keys::F2)) {
        auto* sphere = dynamic_cast<SphereComponent*>(game->Components[1]);
        followingSphere = sphere;
        isFollowing = true;
        interpolation = 0.0f;
    }
    if (game->InputDev->IsKeyDown(Keys::F3)) {
        auto* sphere = dynamic_cast<SphereComponent*>(game->Components[2]);
        followingSphere = sphere;
        isFollowing = true;
        interpolation = 0.0f;
    }
    if (game->InputDev->IsKeyDown(Keys::F4)) {
        auto* sphere = dynamic_cast<SphereComponent*>(game->Components[3]);
        followingSphere = sphere;
        isFollowing = true;
        interpolation = 0.0f;
    }
    if (game->InputDev->IsKeyDown(Keys::F5)) {
        auto* sphere = dynamic_cast<SphereComponent*>(game->Components[4]);
        followingSphere = sphere;
        isFollowing = true;
        interpolation = 0.0f;
    }
}

