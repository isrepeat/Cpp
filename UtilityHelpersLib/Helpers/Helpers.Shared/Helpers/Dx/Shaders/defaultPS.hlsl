//------------------------------------------------------------------------------------
// defaultPS.hlsl
//
// Project = Helpers.Shared;
//
// Simple shader to render a textured quad
//------------------------------------------------------------------------------------

struct Interpolants
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

Texture2D inTexture : register(t0);
SamplerState inSampler : register(s0);

float4 main(Interpolants In) : SV_Target
{
    float4 color = inTexture.Sample(inSampler, In.texcoord);
    return color;
}