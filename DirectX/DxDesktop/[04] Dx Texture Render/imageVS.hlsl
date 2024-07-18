//--------------------------------------------------------------------------------------
// VertexShader.hlsl
//
// Simple vertex shader for rendering a textured quad
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    matrix mWorldViewProj;
};

struct Vertex
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

struct Interpolants
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

Interpolants main(Vertex In)
{
    Interpolants output;
    output.position = mul(In.position, mWorldViewProj);
    output.texcoord = In.texcoord;
    return output;
}

//SamplerState PointSampler : register(s0);
//Texture2D<float4> Texture : register(t0);
//
//struct Interpolators
//{
//    float4 Position : SV_Position;
//    float2 TexCoord : TEXCOORD0;
//};
//
//Interpolators main(uint vI : SV_VertexId)
//{
//    Interpolators output;
//
//    // We use the 'big triangle' optimization so you only Draw 3 verticies instead of 4.
//    float2 texcoord = float2((vI << 1) & 2, vI & 2);
//    output.TexCoord = texcoord;
//    output.Position = float4(texcoord.x * 2 - 1, -texcoord.y * 2 + 1, 0, 1);
//
//    return output;
//}