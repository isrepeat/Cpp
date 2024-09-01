//--------------------------------------------------------------------------------------
// VertexShader.hlsl
//
// Simple vertex shader for rendering a textured quad
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
// This is the default code in the fixed header section.
Texture2DArray inTexture : register(t0);
SamplerState samplerState : register(s0);


export float4 PixelFunctionB(float4 color)
{
    return color * 0.5f;
}