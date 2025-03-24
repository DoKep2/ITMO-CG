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
    float xOffset;
    float yOffset;
    float4x4 mat;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
    input.pos.x += xOffset;
    input.pos.y += yOffset;
    input.pos = mul(input.pos, mat);
	
    output.pos = input.pos;
    output.inTexCoord = input.inTexCoord;
	
    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float3 pixelColor = objTexture.Sample(objSamplerState, input.inTexCoord);
    return float4(pixelColor, 1.0f);
}