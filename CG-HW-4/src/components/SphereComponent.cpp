#include "SphereComponent.h"
#include "GameComponent.h"
#include "Game.h"
#include <d3dcompiler.h>
#include "DDSTextureLoader.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Aboba.h"
#include "Mesh.h"

SphereComponent::SphereComponent(Game *g)
    : GameComponent(g), mat(XMMatrixIdentity())
{
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
    LoadModel("..\\src\\textures\\katamari_ball.obj");
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

void SphereComponent::DestroyResources() {
    layout_->Release();
    pixelShader_->Release();
    pixelShaderByteCode_->Release();
    rastState_->Release();
    vertexShader_->Release();
    vertexShaderByteCode_->Release();
    this->textureSRV->Release();
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

void SphereComponent::Update() {
    if (isGrounded && game->InputDev->IsKeyDown(Keys::Space)) {
        velocity = XMVectorSetY(velocity, jumpStrength);
        isGrounded = false;
    }

    XMVECTOR gravity = XMVectorSet(0, -2.8f, 0, 0);
    velocity = XMVectorAdd(velocity, XMVectorScale(gravity, 0.001f));

    // Обновляем позицию шара
    XMVECTOR newPosition = XMVectorAdd(GetPosition(), XMVectorScale(velocity, 0.001f));
    SetPosition(newPosition);

    // Проверка на "землю" (предположим, что земля на Y = 0)
    if (newPosition.m128_f32[1] <= 0.0f) {
    XMVECTOR correctedPosition = XMVectorSetY(newPosition, 0.0f);
    SetPosition(correctedPosition);
    velocity = XMVectorSetY(velocity, 0.0f);
    isGrounded = true;
    }

    if (velocity.m128_f32[1] > 0) {
        velocity = XMVectorSetY(velocity, velocity.m128_f32[1] * 0.98f); // плавное замедление вверх
    }


    HandleInput();

    XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

    XMVECTOR inputDir = XMVectorZero();

    if (game->InputDev->IsKeyDown(Keys::W)) {
        inputDir = XMVectorAdd(inputDir, forward);
    }
    if (game->InputDev->IsKeyDown(Keys::S)) {
        inputDir = XMVectorSubtract(inputDir, forward);
    }
    if (game->InputDev->IsKeyDown(Keys::A)) {
        inputDir = XMVectorSubtract(inputDir, right);
    }
    if (game->InputDev->IsKeyDown(Keys::D)) {
        inputDir = XMVectorAdd(inputDir, right);
    }

    if (!XMVector3Equal(inputDir, XMVectorZero()))
        inputDir = XMVector3Normalize(inputDir);

    XMVECTOR pos = GetPosition();

    float followDistance = 2.0f;
    float heightOffset = 1.0f;
    float smoothness = 0.5f;
    float deltaTime = 0.016f;
    // game->camera->SmoothFollow(pos, inputDir, deltaTime, followDistance, heightOffset, smoothness);

    game->Context->UpdateSubresource(constantBuffer.Get(), 0, 0, &constantBuffer.data, 0, 0);


    for(auto& c : game->Components)
    {
        if (c != this)
        {
            auto aboba = dynamic_cast<Aboba*>(c);
            if (aboba)
            {
                CheckCollision(aboba);
            }
        }
    }
}

void SphereComponent::CheckCollision(Aboba* aboba) const {
    auto ballPos = GetPosition();
    auto itemPos = aboba->GetPosition();

    BoundingSphere sphere;
    sphere.Center = XMFLOAT3(ballPos.m128_f32[0], ballPos.m128_f32[1], ballPos.m128_f32[2]);
    sphere.Radius = 1.24155295;

    BoundingBox box;
    box.Center = XMFLOAT3(itemPos.m128_f32[0], itemPos.m128_f32[1], itemPos.m128_f32[2]);
    box.Extents = XMFLOAT3(0.330583006f, 0.4085145f, 0.33058351f);

    // Проверка коллизии без учета поворота шара
    if (sphere.Intersects(box) && !aboba->GetIsAttached()) {
        XMVECTOR itemPosVec = itemPos;
        XMVECTOR ballPosVec = ballPos;

        // Смещение в мировой системе координат
        XMVECTOR worldOffset = XMVectorSubtract(itemPosVec, ballPosVec);

        // Нормализуем направление (вектор от центра шара к предмету)
        XMVECTOR directionToItem = XMVector3Normalize(worldOffset);

        // Величина "впивания" внутрь шара (можно настроить)
        float penetrationDepth = 0.0f; // можно экспериментировать с этим значением

        // Делаем шаг назад в сторону центра шара
        XMVECTOR adjustedOffset = XMVectorSubtract(worldOffset, XMVectorScale(directionToItem, penetrationDepth));

        // Сохраняем данные прикрепления (без учета поворота)

        if(!aboba->GetIsAttached()) {
            aboba->SetAttachedInitialRotation(GetRotation());
        }
        aboba->SetAttachedOffset(adjustedOffset);
        aboba->SetIsAttached(true);
        aboba->SetAttachedSphere(this);
    }
}



void SphereComponent::Reload() {
}

SphereComponent::~SphereComponent() {
    DestroyResources();
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


void SphereComponent::HandleInput() {
    float dt = 0.01f;

    XMVECTOR forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

    XMVECTOR inputDir = XMVectorZero();

    if (game->InputDev->IsKeyDown(Keys::W)) {
        inputDir = XMVectorAdd(inputDir, forward);
    }
    if (game->InputDev->IsKeyDown(Keys::S)) {
        inputDir = XMVectorSubtract(inputDir, forward);
    }
    if (game->InputDev->IsKeyDown(Keys::A)) {
        inputDir = XMVectorSubtract(inputDir, right);
    }
    if (game->InputDev->IsKeyDown(Keys::D)) {
        inputDir = XMVectorAdd(inputDir, right);
    }

    if (!XMVector3Equal(inputDir, XMVectorZero())) {
        inputDir = XMVector3Normalize(inputDir);

        float maxSpeed = 0.3f;
        float currentSpeed = XMVectorGetX(XMVector3Length(velocity));
        float speedFactor = 1.0f - std::min(currentSpeed / maxSpeed, 1.0f);
        float actualAccel = acceleration * speedFactor;

        XMVECTOR acc = XMVectorScale(inputDir, actualAccel);
        velocity = XMVectorAdd(velocity, acc);

        heading = atan2f(XMVectorGetX(inputDir), XMVectorGetZ(inputDir));
    }

    velocity = XMVectorScale(velocity, friction);

    XMVECTOR scale, rotQuat, trans;
    XMMatrixDecompose(&scale, &rotQuat, &trans, mat);
    trans = XMVectorAdd(trans, velocity);

    XMVECTOR moveDir = XMVector3Normalize(velocity);
    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMVECTOR rollAxis = XMVector3Cross(moveDir, up);

    float moveLen = XMVectorGetX(XMVector3Length(velocity));
    if (moveLen > 0.0001f && !XMVector3Equal(rollAxis, XMVectorZero())) {
        float rollAngle = moveLen;
        XMVECTOR rollQuat = XMQuaternionRotationAxis(-rollAxis, rollAngle);
        rotQuat = XMQuaternionMultiply(rotQuat, rollQuat);
    }

    // Пересоберем world-матрицу с новой ориентацией
    mat = XMMatrixScalingFromVector(scale) * XMMatrixRotationQuaternion(rotQuat) * XMMatrixTranslationFromVector(trans);
}


void SphereComponent::SetWorldMatrix(const XMMATRIX& matrix) {
    mat = matrix;
}

XMMATRIX SphereComponent::GetWorldMatrix() const {
    return mat;
}

bool SphereComponent::LoadModel(const std::string& modelPath)
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

std::wstring ToWide(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

std::string GetDirectoryFrom(const std::string& fullPath) {
    size_t slashIndex = fullPath.find_last_of("/\\");
    return (slashIndex == std::string::npos) ? "." : fullPath.substr(0, slashIndex);
}


void SphereComponent::ProcessNode(aiNode *node, const aiScene *scene, const std::string& modelPath) {
    for (UINT i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        // Обрабатываем меш
        meshes_.push_back(ProcessMesh(mesh, scene));

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        aiString texturePath;
        bool hasTexture = material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS;

        if (hasTexture) {
            std::string texFilename = texturePath.C_Str();
            std::string fullPath = GetDirectoryFrom(modelPath) + "/" + texFilename;

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
        auto* newMaterial = new ConstantBuffer<Material>();
        newMaterial->Initialize(game->Device.Get(), game->Context);

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
        newMaterial->data = materialData;
        materials_.push_back(*newMaterial);  // Добавляем материал в вектор
    }

    for (UINT i = 0; i < node->mNumChildren; i++) {
        this->ProcessNode(node->mChildren[i], scene, modelPath);
    }
}


Mesh SphereComponent::ProcessMesh(aiMesh *mesh, const aiScene *scene) {
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


void SphereComponent::SetPosition(const XMFLOAT3& newPosition) {
    XMVECTOR scale, rotation, translation;
    XMMatrixDecompose(&scale, &rotation, &translation, mat);

    XMVECTOR posVec = XMLoadFloat3(&newPosition);
    mat = XMMatrixScalingFromVector(scale) *
            XMMatrixRotationQuaternion(rotation) *
            XMMatrixTranslationFromVector(posVec);
}

void SphereComponent::SetPosition(const XMVECTOR& newPosition) {
    SetPosition(XMFLOAT3(newPosition.m128_f32[0], newPosition.m128_f32[1], newPosition.m128_f32[2]));
}

void SphereComponent::SetRotation(const XMFLOAT4& newRotationQuat) {

    XMVECTOR scale, rotation, translation;
    XMMatrixDecompose(&scale, &rotation, &translation, mat);

    XMVECTOR rotVec = XMLoadFloat4(&newRotationQuat);
    mat = XMMatrixScalingFromVector(scale) *
            XMMatrixRotationQuaternion(rotVec) *
            XMMatrixTranslationFromVector(translation);
}
