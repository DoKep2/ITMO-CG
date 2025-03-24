#include "SphereComponent.h"
#include "GameComponent.h"
#include "Game.h"
#include <d3dcompiler.h>
#include "DDSTextureLoader.h"
#include <iostream>

float SmoothStep(float t);

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
                std::cout << "ABOBA";
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

    auto res = D3DCompileFromFile(L"C:\\Users\\sergo\\source\\repos\\CG-HW-3\\CG-HW-1\\TextureShader.hlsl",
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
            // MessageBox(game->Display->hWnd, L"TextureShader.hlsl", L"Missing Shader File", MB_OK);
        }

        return;
    }

    D3D_SHADER_MACRO Shader_Macros[] = {{"TEXCOORD", "float2"}, {nullptr, nullptr}};

    ID3DBlob *errorPixelCode;
    res = D3DCompileFromFile(L"C:\\Users\\sergo\\source\\repos\\CG-HW-3\\CG-HW-1\\TextureShader.hlsl",
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
    // rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FillMode = D3D11_FILL_WIREFRAME;

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
        // MessageBox(0, L"Texture not found", L"Error", MB_OK);
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
    constantBuffer.data.xOffset = offset.x;
    constantBuffer.data.yOffset = offset.y;
    /*XMMATRIX world = DirectX::XMMatrixIdentity();
    static DirectX::XMVECTOR eyePos = DirectX::XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f);
    DirectX::XMFLOAT3 eyePosFloat3;
    DirectX::XMStoreFloat3(&eyePosFloat3, eyePos);
    eyePosFloat3.y += 0.01f;
    eyePos = DirectX::XMLoadFloat3(&eyePosFloat3);
    static DirectX::XMVECTOR lookAtPos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    static DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, lookAtPos, upVector);
    float fovDegrees = 90.0f;
    float fovRadians = (fovDegrees / 360.0f) * DirectX::XM_2PI;
    float aspectRatio = static_cast<float>(viewport.Width) / static_cast<float>(viewport.Height);
    float nearZ = 0.1f;
    float farZ = 1000.0f;
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);*/
    //constantBuffer.data.mat = mat * view * projectionMatrix;
    constantBuffer.data.mat = mat * camera.GetViewMatrix() * camera.GetProjectionMatrix();
    constantBuffer.data.mat = XMMatrixTranspose(constantBuffer.data.mat);
    if (!constantBuffer.ApplyChanges()) {
        return;
    }
    game->Context->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());
    game->Context->DrawIndexed(std::size(indices_), 0, 0);
}

void SphereComponent::Update() {
    static float cameraSpeed = 0.01f;
    RotateByCenter(orbitalVelocity);
    // game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &position_, 0, 0);
    // game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &mat, 0, 0);
    game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &constantBuffer.data, 0, 0);
    //set camera rotation by mouse


    if (game->InputDev->IsKeyDown(Keys::W)) {
        camera.AdjustPosition(camera.GetForwardVector() * cameraSpeed);
    }

    if (game->InputDev->IsKeyDown(Keys::A)) {
        camera.AdjustPosition(camera.GetLeftVector() * cameraSpeed);
    }

    if (game->InputDev->IsKeyDown(Keys::S)) {
        camera.AdjustPosition(camera.GetBackwardVector() * cameraSpeed);
    }

    if (game->InputDev->IsKeyDown(Keys::D)) {
        camera.AdjustPosition(camera.GetRightVector() * cameraSpeed);
    }

    if (game->InputDev->IsKeyDown(Keys::Space)) {
        camera.AdjustPosition(0.0f, cameraSpeed, 0.0f);
    }
    if (game->InputDev->IsKeyDown(Keys::LeftShift)) {
        camera.AdjustPosition(0.0f, -cameraSpeed, 0.0f);
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

    // Глобальные переменные
    static bool isAnimating = false;  // Флаг анимации
    static float t = 0.0f;            // Прогресс интерполяции (0-1)

    auto basePos = XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f);
    auto baseRot = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

    if (game->InputDev->IsKeyDown(Keys::F1)) {
        isAnimating = true;  // Запускаем анимацию
        t = 0.0f;            // Обнуляем интерполяцию
    }

    if (game->InputDev->IsKeyDown(Keys::F2)) {
        isAnimating = true;  // Запускаем анимацию
        t = 0.0f;            // Обнуляем интерполяцию
        const auto* sphere = dynamic_cast<SphereComponent*>(game->Components[1]);
        basePos = XMVectorSet(sphere->position_.x, sphere->position_.y, -2.0f, 0.0f);
    }

    if (isAnimating) {
        auto currentPos = camera.GetPositionVector();
        auto currentRot = camera.GetRotationVector();

        // Увеличиваем t (чем меньше шаг, тем плавнее анимация)
        t += 0.001f;
        if (t > 1.0f) {
            t = 1.0f;
            isAnimating = false; // Завершаем анимацию
        }

        float smoothT = SmoothStep(t);  // Применяем сглаживание
        auto newPos = XMVectorLerp(currentPos, basePos, smoothT);
        auto newRot = XMVectorLerp(currentRot, baseRot, smoothT);
        camera.SetPosition(newPos);
        camera.SetRotation(newRot);
    }

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
    mat = XMMatrixRotationY(rotationAngle);
}

void SphereComponent::SetPosition(float x, float y, float z) {
    position_ = XMFLOAT3(x, y, z);
    mat = XMMatrixTranslation(x, y, 0.0f);
    offset = SimpleMath::Vector4(x, y, z, 0.0f);
}


