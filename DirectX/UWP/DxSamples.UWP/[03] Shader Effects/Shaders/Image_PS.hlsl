//------------------------------------------------------------------------------------
// PixelShader.hlsl
//
// Simple shader to render a textured quad
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
// This is the default code in the fixed header section.
Texture2D Texture : register(t0);
SamplerState Anisotropic : register(s0);

struct Interpolants
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};


float4 main(Interpolants In) : SV_Target
{
    float4 color = Texture.Sample(Anisotropic, In.texcoord);
    //color = float4(0.5f, 0.0f, 0.0f, 1.0f);
    return color;
}