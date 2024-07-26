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