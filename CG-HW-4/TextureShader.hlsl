// Структура входных данных для вершинного шейдера
struct VS_IN
{
    float4 pos : POSITION0;         // Позиция вершины
    float2 inTexCoord : TEXCOORD;   // Координаты текстуры
};

// Структура выходных данных для пиксельного шейдера
struct PS_IN
{
    float4 pos : SV_POSITION;       // Преобразованная позиция
    float2 inTexCoord : TEXCOORD;   // Координаты текстуры
};

// Константный буфер для передачи данных о камере и объекте
cbuffer cbPerObject : register(b0)
{
    float4x4 world;       // Матрица мирового преобразования
    float4x4 view;        // Матрица камеры
    float4x4 projection;  // Матрица проекции
};

// Материальные данные
cbuffer cbMaterial : register(b1)
{
    float3 diffuseColor;   // 12 байт
    float  opacity;        // 4 байта

    float3 ambientColor;   // 12 байт
    float  hasTexture;     // 4 байта (bool нельзя напрямую, используем float или int)

    float3 specularColor;  // 12 байт
    float  pad;            // 4 байта для выравнивания
};

// Текстура объекта и самплер для текстуры
Texture2D objTexture : register(t0);
SamplerState objSamplerState : register(s0);

// Вершинный шейдер
PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN)0;

    // Преобразуем позицию вершины
    input.pos = mul(input.pos, world);
    input.pos = mul(input.pos, view);
    input.pos = mul(input.pos, projection);

    output.pos = input.pos;
    output.inTexCoord = input.inTexCoord;

    return output;
}

// Пиксельный шейдер
float4 PSMain(PS_IN input) : SV_Target
{
    float3 finalColor;

    if (hasTexture)
    {
        float4 textureColor = objTexture.Sample(objSamplerState, input.inTexCoord);
        finalColor = textureColor.rgb;
    }
    else
    {
        finalColor = diffuseColor;
    }

    return float4(finalColor, 1.0f);
    //return float4(0.5f, 0.5f, 0.5f, 1.0f);
}

