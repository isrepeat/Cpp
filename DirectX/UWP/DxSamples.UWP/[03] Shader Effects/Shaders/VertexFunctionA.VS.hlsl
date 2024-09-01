//--------------------------------------------------------------------------------------
// VertexShader.hlsl
//
// Simple vertex shader for rendering a textured quad
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
// This is the default code in the fixed header section.
//Texture2D Texture : register(t0);
//SamplerState Anisotropic : register(s0);


export void VertexFunctionA(inout float4 position, inout float2 uv)
{
    position.x += 0.3;
}