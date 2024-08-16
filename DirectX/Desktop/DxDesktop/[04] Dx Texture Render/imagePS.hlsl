//------------------------------------------------------------------------------------
// PixelShader.hlsl
//
// Simple shader to render a textured quad
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

struct Interpolants
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

struct Pixel
{
    float4 color : SV_Target;
};

Texture2D txDiffuse : register(t0);
SamplerState pointSampler : register(s0);

Pixel main(Interpolants In)
{
    Pixel Out;
    Out.color = txDiffuse.Sample(pointSampler, In.texcoord);
    return Out;
}