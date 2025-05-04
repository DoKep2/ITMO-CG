struct VS_IN
{
    float4 pos : POSITION0;
    float2 inTexCoord : TEXCOORD;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;

    input.pos = mul(input.pos, world);
    input.pos = mul(input.pos, view);
    input.pos = mul(input.pos, projection);

    output.pos = input.pos;
    output.inTexCoord = input.inTexCoord;
	
    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float3 pixelColor = objTexture.Sample(objSamplerState, input.inTexCoord);
    return float4(pixelColor, 1.0f);
}