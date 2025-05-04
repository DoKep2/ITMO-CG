#include <assimp/Importer.hpp>      // Для импорта моделей
#include <assimp/scene.h>           // Для данных сцены
#include <assimp/postprocess.h>     // Для обработки данных

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <iostream>

// Структура для хранения вершин
struct Vertex
{
    DirectX::XMFLOAT3 position;  // Позиция вершины
    DirectX::XMFLOAT3 normal;    // Нормаль вершины
    DirectX::XMFLOAT2 texCoord;  // Текстурные координаты
};

// Функция для загрузки модели с помощью Assimp
bool LoadModel(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    Assimp::Importer importer;

    // Загрузка модели с обработкой ошибок
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene)
    {
        std::cerr << "Error loading model: " << importer.GetErrorString() << std::endl;
        return false;
    }

    // Предполагаем, что модель состоит из одного меша
    aiMesh* mesh = scene->mMeshes[0];

    // Перебор вершин
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        // Позиция
        vertex.position = DirectX::XMFLOAT3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        // Нормали
        if (mesh->HasNormals())
        {
            vertex.normal = DirectX::XMFLOAT3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        }

        // Текстурные координаты
        if (mesh->HasTextureCoords(0))
        {
            vertex.texCoord = DirectX::XMFLOAT2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        }

        vertices.push_back(vertex);
    }

    // Перебор индексов
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    return true;
}
