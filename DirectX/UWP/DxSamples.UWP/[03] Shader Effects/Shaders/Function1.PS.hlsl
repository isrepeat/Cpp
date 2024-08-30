//------------------------------------------------------------------------------------
// PixelShader.hlsl
//
// Simple shader to render a textured quad
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
// This is the default code in the fixed header section.
//Texture2D<float3> Texture : register(t0);
//SamplerState Anisotropic : register(s0);

//float4 Function1(float4 color : COLOR) : SV_Target
//{
//    //return color * 0.5f;
//    return float4(0.5f, 0.0f, 0.0f, 1.0f);
//}




//Texture2DArray inTexture : register(t0);
//SamplerState samplerState : register(s0);

//cbuffer CameraData : register(b0)
//{
//    float4x4 Model;
//    float4x4 View;
//    float4x4 Projection;
//};

//export void VertexFunction(inout float4 position, inout float2 uv)
//{
//    position = mul(position, Model);
//    position = mul(position, View);
//    position = mul(position, Projection);
//}

//export float4 SampleTexture(float2 uv)
//{
//    return inTexture.Sample(samplerState, float3(uv, 0));
//}