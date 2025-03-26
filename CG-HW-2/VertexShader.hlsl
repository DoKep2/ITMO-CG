struct VS_IN
{
    float4 pos : POSITION0;
    float4 col : COLOR0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};


PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;

    input.pos = mul(input.pos, world);
    input.pos = mul(input.pos, view);
    input.pos = mul(input.pos, projection);

    output.pos = input.pos;
    output.col = input.col;

    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float4 col = input.col;
    return col;
}
