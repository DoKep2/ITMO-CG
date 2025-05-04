#include "PlaneComponent.h"
#include "GameComponent.h"
#include "Game.h"
#include <d3dcompiler.h>
#include "DDSTextureLoader.h"
#include <iostream>

PlaneComponent::PlaneComponent(Game *g, std::wstring texturePath, float width, float depth)
    : GameComponent(g), mat(XMMatrixIdentity()), texturePath(texturePath), width_(width), depth_(depth)
{
    Vertex verticesTmp[] = {
        //       POSITION                                TEXCOORD
        { {-width_, -0.6f, -depth_, 1.0f},                {0.0f, 1.0f} },
        { { width_, -0.6f, -depth_, 1.0f},                {1.0f, 1.0f} },
        { { width_, -0.6f,  depth_, 1.0f},                {1.0f, 0.0f} },
        { {-width_, -0.6f,  depth_, 1.0f},                {0.0f, 0.0f} },
    };
    std::memcpy(vertices_, verticesTmp, sizeof(verticesTmp));

    // std::memcpy(points_, pointsTmp, sizeof(pointsTmp));

    int indicesTmp[] = {
        0, 1, 2,
        0, 2, 3
    };
    std::memcpy(indices_, indicesTmp, sizeof(indicesTmp));
}

void PlaneComponent::Initialize() {
    ID3DBlob *errorVertexCode = nullptr;

    auto res = D3DCompileFromFile(L"..\\aboba.hlsl",
                                  nullptr,
                                  nullptr,
                                  "VSMain",
                                  "vs_5_0",
                                  D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
                                  0,
                                  vertexShaderByteCode_.GetAddressOf(),
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
    res = D3DCompileFromFile(L"..\\aboba.hlsl",
                             Shader_Macros,
                             nullptr,
                             "PSMain",
                             "ps_5_0",
                             D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
                             0,
                             pixelShaderByteCode_.GetAddressOf(),
                             &errorPixelCode);

    game->Device->CreateVertexShader(
        vertexShaderByteCode_->GetBufferPointer(),
        vertexShaderByteCode_->GetBufferSize(),
        nullptr, vertexShader_.GetAddressOf());

    game->Device->CreatePixelShader(
        pixelShaderByteCode_->GetBufferPointer(),
        pixelShaderByteCode_->GetBufferSize(),
        nullptr, pixelShader_.GetAddressOf());

    // D3D11_INPUT_ELEMENT_DESC inputElements[] = {
    //     D3D11_INPUT_ELEMENT_DESC{
    //         "POSITION",
    //         0,
    //         DXGI_FORMAT_R32G32B32A32_FLOAT,
    //         0,
    //         0,
    //         D3D11_INPUT_PER_VERTEX_DATA,
    //         0
    //     },
    //     D3D11_INPUT_ELEMENT_DESC{
    //         "TEXCOORD",
    //         0,
    //         DXGI_FORMAT_R32G32B32A32_FLOAT,
    //         0,
    //         D3D11_APPEND_ALIGNED_ELEMENT,
    //         D3D11_INPUT_PER_VERTEX_DATA,
    //         0
    //     }
    // };
    D3D11_INPUT_ELEMENT_DESC inputElements[] = {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
            0, D3D11_INPUT_PER_VERTEX_DATA, 0
        },
        {
            "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
        }
    };


    game->Device->CreateInputLayout(
        inputElements,
        2,
        vertexShaderByteCode_->GetBufferPointer(),
        vertexShaderByteCode_->GetBufferSize(),
        layout_.GetAddressOf());

    D3D11_BUFFER_DESC vertexBufDesc = {};
    //vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    //vertexBufDesc.CPUAccessFlags = 0;
    vertexBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;
    // vertexBufDesc.ByteWidth = sizeof(SimpleMath::Vector4) * std::size(points_);
    vertexBufDesc.ByteWidth = sizeof(Vertex) * std::size(vertices_);


    D3D11_SUBRESOURCE_DATA vertexData = {};
    // vertexData.pSysMem = points_;
    vertexData.pSysMem = vertices_;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    std::cout << "device ptr = " << std::hex << reinterpret_cast<uintptr_t>(game->Device.Get()) << std::endl;
    std::cout << "bufferDesc.ByteWidth = " << vertexBufDesc.ByteWidth << std::endl;
    std::cout << "initData.pSysMem = " << vertexData.pSysMem << std::endl;

    game->Device->CreateBuffer(&vertexBufDesc, &vertexData, vb_.GetAddressOf());
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



    game->Device->CreateBuffer(&indexBufDesc, &indexData, ib_.GetAddressOf());

    // strides_[0] = 32;
    strides_[0] = sizeof(Vertex);
    offsets_[0] = 0;

    HRESULT hr = this->constantBuffer.Initialize(game->Device.Get(), game->Context);
    if (FAILED(hr)) {
        return;
    }

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    // rastDesc.FillMode = D3D11_FILL_WIREFRAME;

    res = game->Device->CreateRasterizerState(&rastDesc, rastState_.GetAddressOf());

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
        textureSRV_.GetAddressOf()
    );

    if (FAILED(hr)) {
        MessageBox(0, L"Texture not found", L"Error", MB_OK);
    }
}

void PlaneComponent::DestroyResources() {
    layout_->Release();
    pixelShader_->Release();
    pixelShaderByteCode_->Release();
    rastState_->Release();
    vertexShader_->Release();
    vertexShaderByteCode_->Release();
    vb_->Release();
    ib_->Release();
    textureSRV_->Release();
}

void PlaneComponent::Draw() {
    game->Context->RSSetState(rastState_.Get());
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(game->Display->ClientWidth);
    viewport.Height = static_cast<float>(game->Display->ClientHeight);
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;
    game->Context->RSSetViewports(1, &viewport);
    game->Context->IASetInputLayout(layout_.Get());
    game->Context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    game->Context->IASetIndexBuffer(ib_.Get(), DXGI_FORMAT_R32_UINT, 0);
    game->Context->IASetVertexBuffers(0, 1, vb_.GetAddressOf(), strides_, offsets_);
    game->Context->VSSetShader(vertexShader_.Get(), nullptr, 0);
    game->Context->PSSetShader(pixelShader_.Get(), nullptr, 0);
    game->Context->PSSetSamplers(0, 1, samplerState.GetAddressOf());
    game->Context->PSSetShaderResources(0, 1, textureSRV_.GetAddressOf());

    constantBuffer.data.world = XMMatrixTranspose(mat);
    constantBuffer.data.view = XMMatrixTranspose(game->camera->GetViewMatrix());
    constantBuffer.data.projection = XMMatrixTranspose(game->camera->GetProjectionMatrix());

    if (!constantBuffer.ApplyChanges()) {
        return;
    }
    game->Context->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());
    game->Context->DrawIndexed(std::size(indices_), 0, 0);
}

void PlaneComponent::Update() {

    game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &constantBuffer.data, 0, 0);
}

void PlaneComponent::Reload() {
}

PlaneComponent::~PlaneComponent() {
    DestroyResources();
}
