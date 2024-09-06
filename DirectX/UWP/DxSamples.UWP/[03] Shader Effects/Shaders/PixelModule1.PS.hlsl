Texture2DArray inTexture : register(t0);
SamplerState samplerState : register(s0);

export float4 PixelFunctionA(float4 position, float2 uv)
{
    return inTexture.Sample(samplerState, float3(uv, 0));
}