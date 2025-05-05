#include "Aboba.h"
#include "GameComponent.h"
#include "Game.h"
#include <d3dcompiler.h>
#include <fstream>

#include "DDSTextureLoader.h"
#include <iostream>
#include <SphereComponent.h>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Mesh.h"

Aboba::Aboba(Game *g, std::wstring InModelPath)
    : GameComponent(g), mat(XMMatrixIdentity()), modelPath(InModelPath)
{

}

void Aboba::Initialize() {
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
    LoadModel(std::string(modelPath.begin(), modelPath.end()));
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
            // DXGI_FORMAT_R32G32B32A32_FLOAT,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        D3D11_INPUT_ELEMENT_DESC{
            "TEXCOORD",
            0,
            // DXGI_FORMAT_R32G32B32A32_FLOAT,
            DXGI_FORMAT_R32G32_FLOAT,
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

    strides_[0] = sizeof(Vertex2);
    offsets_[0] = 0;

    HRESULT hr = this->constantBuffer.Initialize(game->Device.Get(), game->Context);
    if (FAILED(hr)) {
        return;
    }

    hr = this->material.Initialize(game->Device.Get(), game->Context);
    if (FAILED(hr)) {
        return;
    }
    CD3D11_RASTERIZER_DESC rastDesc = {};
    // rastDesc.CullMode = D3D11_CULL_FRONT;
    rastDesc.CullMode = D3D11_CULL_NONE;
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
}

void Aboba::DestroyResources() {
    layout_->Release();
    pixelShader_->Release();
    pixelShaderByteCode_->Release();
    rastState_->Release();
    vertexShader_->Release();
    vertexShaderByteCode_->Release();
    this->textureSRV->Release();
}

void Aboba::Draw() {
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
    game->Context->VSSetShader(vertexShader_, nullptr, 0);
    game->Context->PSSetShader(pixelShader_, nullptr, 0);
    game->Context->PSSetSamplers(0, 1, samplerState.GetAddressOf());
    if(this->textureSRV) game->Context->PSSetShaderResources(0, 1, &this->textureSRV);

    constantBuffer.data.world = XMMatrixTranspose(mat);
    constantBuffer.data.view = XMMatrixTranspose(game->camera->GetViewMatrix());
    constantBuffer.data.projection = XMMatrixTranspose(game->camera->GetProjectionMatrix());

    if (!constantBuffer.ApplyChanges()) {
        return;
    }

    game->Context->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());
    for (size_t i = 0; i < meshes_.size(); ++i) {
        ConstantBuffer<Material> material;
        material = materials_[i];
        if(!material.ApplyChanges()) {
            return;
        }
        if (material.data.hasTexture > 0.5f && textures_[i] != nullptr) {
        game->Context->PSSetShaderResources(0, 1, textures_[i].GetAddressOf());
        } else {
            game->Context->PSSetShaderResources(0, 0, nullptr);
        }

        game->Context->UpdateSubresource(material.Get(), 0, nullptr, &material.data, 0, 0);
        game->Context->PSSetConstantBuffers(1, 1, material.GetAddressOf());
        game->Context->VSSetConstantBuffers(1, 1, material.GetAddressOf());

        meshes_[i].Draw();
    }
}

void Aboba::Update() {
    if (isAttached) {
        // Поворот шара на текущем кадре
        XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(GetRotation());

        // Преобразуем локальный offset обратно в мировую систему
        XMVECTOR rotatedOffset = XMVector3Transform(attachedOffset, rotationMatrix);

        // Вычисляем новую мировую позицию предмета
        XMVECTOR newWorldPos = XMVectorAdd(attachedSphere->GetPosition(), rotatedOffset);
        SetPosition(XMFLOAT3(
            newWorldPos.m128_f32[0],
            newWorldPos.m128_f32[1],
            newWorldPos.m128_f32[2]
        ));

        XMVECTOR rot = XMQuaternionMultiply(XMQuaternionInverse(attachedInitialRotation), attachedSphere->GetRotation());

        SetRotation(XMFLOAT4(rot.m128_f32[0], rot.m128_f32[1], rot.m128_f32[2], rot.m128_f32[3]));
    }

    game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &constantBuffer.data, 0, 0);
}


void Aboba::SetAttachedInitialRotation(XMVECTOR rot) {
    attachedInitialRotation = rot;
}

void Aboba::Reload() {
}

Aboba::~Aboba() {
    DestroyResources();
}

XMVECTOR Aboba::GetPosition() const {
    XMVECTOR scale, rotQuat, trans;
    XMMatrixDecompose(&scale, &rotQuat, &trans, mat);
    return trans;
}

XMVECTOR Aboba::GetRotation() const {
    XMVECTOR scale, rotQuat, trans;
    XMMatrixDecompose(&scale, &rotQuat, &trans, mat);
    return rotQuat;
}

std::vector<XMFLOAT3> Aboba::GetVertices() const {
    return vertices_;
}

bool Aboba::GetIsAttached() const {
    return isAttached;
}

void Aboba::SetIsAttached(bool InIsAttached) {
    isAttached = InIsAttached;
}

void Aboba::SetAttachedSphere(const SphereComponent *sphere) {
    attachedSphere = sphere;
}

void Aboba::SetAttachedOffset(XMVECTOR offset) {
    attachedOffset = offset;
}


void Aboba::SetWorldMatrix(const XMMATRIX& matrix) {
    mat = matrix;
}

XMMATRIX Aboba::GetWorldMatrix() const {
    return mat;
}

bool Aboba::LoadModel(const std::string& modelPath)
{
    // Шаг 1: Используем Assimp для импорта модели
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (!scene)
    {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        return false;
    }

    ProcessNode(scene->mRootNode, scene, modelPath);
    return true;
}

std::wstring Aboba::ToWide(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

std::string Aboba::GetDirectoryFrom(const std::string& fullPath) {
    size_t slashIndex = fullPath.find_last_of("/\\");
    return (slashIndex == std::string::npos) ? "." : fullPath.substr(0, slashIndex);
}


void Aboba::ProcessNode(aiNode *node, const aiScene *scene, const std::string& modelPath) {
    for (UINT i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        // Обрабатываем меш
        meshes_.push_back(ProcessMesh(mesh, scene));

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        aiString texturePath;
        bool hasTexture = material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS;

        if (hasTexture) {
            std::string texFilename = texturePath.C_Str();
            // std::string fullPath = GetDirectoryFrom(modelPath) + "\\" + texFilename;
            std::string fullPath = "..\\src\\textures\\Muchkin2_baseColor.dds";
            std::ifstream file(fullPath);
            bool a = file.good();
            // Загружаем текстуру
            HRESULT hr = DirectX::CreateDDSTextureFromFile(
                game->Device.Get(),
                ToWide(fullPath).c_str(),
                nullptr,
                &this->textureSRV
            );
            textures_.emplace_back(this->textureSRV);  // Добавляем текстуру в вектор
        } else {
            this->textureSRV = nullptr;
            textures_.emplace_back(nullptr);  // Нет текстуры, добавляем nullptr
        }

        // Создаем материал для данного меша
        ConstantBuffer<Material> newMaterial;
        newMaterial.Initialize(game->Device.Get(), game->Context);

        Material materialData{};
        aiColor3D diffuseColor(0.f, 0.f, 0.f);
        aiColor3D ambientColor(0.f, 0.f, 0.f);
        aiColor3D specularColor(0.f, 0.f, 0.f);

        // Получаем диффузный цвет (Kd)
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
            materialData.diffuseColor = { diffuseColor.r, diffuseColor.g, diffuseColor.b };
        }

        // Получаем амбиентный цвет (Ka)
        if (material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS) {
            materialData.ambientColor = { ambientColor.r, ambientColor.g, ambientColor.b };
        }

        // Получаем зеркальный цвет (Ks)
        if (material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
            materialData.specularColor = { specularColor.r, specularColor.g, specularColor.b };
        }

        // Получаем прозрачность
        float opacity = 1.0f;
        material->Get(AI_MATKEY_OPACITY, opacity);
        materialData.opacity = opacity;

        // Устанавливаем флаг для текстуры
        materialData.hasTexture = hasTexture ? 1.0f : 0.0f;
        newMaterial.data = materialData;
        materials_.push_back(newMaterial);  // Добавляем материал в вектор
    }

    for (UINT i = 0; i < node->mNumChildren; i++) {
        this->ProcessNode(node->mChildren[i], scene, modelPath);
    }
}


Mesh Aboba::ProcessMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex2> vertices;
    std::vector<uint32_t> indices;

    for(UINT i = 0; i < mesh->mNumVertices; ++i) {
        Vertex2 vertex{};
        vertex.position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        if(mesh->HasTextureCoords(0)) {
            vertex.texCoord = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        vertices.push_back(vertex);
        // vertices_.push_back(vertex.position);
    }

    for(UINT i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for(UINT j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }
    return Mesh{game->Device, game->Context, vertices, indices};
}

void Aboba::SetAttachedDirection(XMVECTOR attachedDirection) {
    this->attachedDirection = attachedDirection;
}

void Aboba::SetAttachedDistance(float attachedDistance) {
    this->attachedDistance = attachedDistance;
}


void Aboba::SetPosition(const XMFLOAT3& newPosition) {
    XMVECTOR scale, rotation, translation;
    XMMatrixDecompose(&scale, &rotation, &translation, mat);

    XMVECTOR posVec = XMLoadFloat3(&newPosition);
    mat = XMMatrixScalingFromVector(scale) *
            XMMatrixRotationQuaternion(rotation) *
            XMMatrixTranslationFromVector(posVec);
}

void Aboba::SetScale(const XMFLOAT3& newScale) {
    XMVECTOR scale, rotation, translation;
    XMMatrixDecompose(&scale, &rotation, &translation, mat);

    XMVECTOR scaleVec = XMLoadFloat3(&newScale);
    mat = XMMatrixScalingFromVector(scaleVec) *
            XMMatrixRotationQuaternion(rotation) *
            XMMatrixTranslationFromVector(translation);
}

void Aboba::SetRotation(const XMFLOAT4& newRotationQuat) {

    XMVECTOR scale, rotation, translation;
    XMMatrixDecompose(&scale, &rotation, &translation, mat);

    XMVECTOR rotVec = XMLoadFloat4(&newRotationQuat);
    mat = XMMatrixScalingFromVector(scale) *
            XMMatrixRotationQuaternion(rotVec) *
            XMMatrixTranslationFromVector(translation);
}
