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

SphereComponent::SphereComponent(Game *g, std::wstring texturePath, float radius)
    : GameComponent(g), mat(XMMatrixIdentity()), texturePath(texturePath), radius_(radius)
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
    UpdateOrbit(0.01f);
    HandleCameraInput();

    static XMVECTOR basePos;
    static XMVECTOR baseRot;

    if (isFollowing) {
        auto followingSpherePos = followingSphere->GetPosition();
        auto followingSphereRot = followingSphere->GetRotation();
        basePos = XMVectorSet(followingSpherePos.m128_f32[0], followingSpherePos.m128_f32[1], followingSpherePos.m128_f32[2] - 0.5 - this->radius_, 0.0f);
        baseRot = XMVectorSet(followingSphereRot.m128_f32[0], followingSphereRot.m128_f32[1], followingSphereRot.m128_f32[2], 0.0f);
        auto currentPos = camera.GetPositionVector();

        interpolation += 0.001f;
        if (interpolation > 1.0f) {
            interpolation = 1.0f;
        }

        float smoothT = SmoothStep(interpolation);
        auto newPos = XMVectorLerp(currentPos, basePos, smoothT);
        camera.SetPosition(newPos);
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

void SphereComponent::UpdateOrbit(float deltaTime) {
    if (!orbitOffsetInitialized)
    {
        orbitOffset = GetPosition() - orbitingTarget->GetPosition();
        orbitOffsetInitialized = true;
    }

    orbitAngle += orbitSpeed * deltaTime;
    selfRotationAngle += selfRotationSpeed * deltaTime;

    XMMATRIX orbitRotation = XMMatrixRotationY(orbitAngle);
    XMVECTOR rotatedOffset = XMVector3Transform(orbitOffset, orbitRotation);

    XMVECTOR targetPos = orbitingTarget->GetPosition();
    XMVECTOR newPos = targetPos + rotatedOffset;

    SetPosition(XMFLOAT3(newPos.m128_f32[0], newPos.m128_f32[1], newPos.m128_f32[2]));

    XMMATRIX selfRotation = XMMatrixRotationY(selfRotationAngle);

    mat = selfRotation * XMMatrixTranslationFromVector(newPos);
}

XMVECTOR SphereComponent::GetPosition() const {
    XMVECTOR scale, rotQuat, trans;
    XMMatrixDecompose(&scale, &rotQuat, &trans, mat);
    return trans;
}

XMVECTOR SphereComponent::GetRotation() const {
    XMVECTOR scale, rotQuat, trans;
    XMMatrixDecompose(&scale, &rotQuat, &trans, mat);
    return rotQuat;
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

void SphereComponent::SetOrbitingTarget(SphereComponent* target, float radius, float speed) {
    orbitingTarget = target;
    orbitRadius = radius;
    orbitSpeed = speed;
    orbitAngle = 0.0f;
}

void SphereComponent::SetWorldMatrix(const XMMATRIX& matrix) {
    mat = matrix;
}

XMMATRIX SphereComponent::GetWorldMatrix() const {
    return mat;
}

void SphereComponent::SetPosition(const XMFLOAT3& newPosition) {
    XMVECTOR scale, rotation, translation;
    XMMatrixDecompose(&scale, &rotation, &translation, mat);

    XMVECTOR posVec = XMLoadFloat3(&newPosition);
    mat = XMMatrixScalingFromVector(scale) *
            XMMatrixRotationQuaternion(rotation) *
            XMMatrixTranslationFromVector(posVec);
}

void SphereComponent::SetRotation(const XMFLOAT4& newRotationQuat) {
    // Разбираем world
    XMVECTOR scale, rotation, translation;
    XMMatrixDecompose(&scale, &rotation, &translation, mat);

    XMVECTOR rotVec = XMLoadFloat4(&newRotationQuat);
    mat = XMMatrixScalingFromVector(scale) *
            XMMatrixRotationQuaternion(rotVec) *
            XMMatrixTranslationFromVector(translation);
}
