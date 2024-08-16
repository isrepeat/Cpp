//--------------------------------------------------------------------------------------
// VertexShader.hlsl
//
// Simple vertex shader for rendering a triangle
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

struct Vertex
{
    float4 position     : SV_Position;
    float4 color        : COLOR0;
};

struct Interpolants
{
    float4 position     : SV_Position;
    float4 color        : COLOR0;
};

Interpolants main(Vertex In)
{
    return In;
}


//cbuffer ModelViewProjectionConstantBuffer : register(b0)
//{
//	matrix model;
//	matrix view;
//	matrix projection;
//};
//
//struct VertexShaderInput
//{
//	float3 pos : POSITION;
//	float3 color : COLOR0;
//};
//
//struct PixelShaderInput
//{
//	float4 pos : SV_POSITION;
//	float3 color : COLOR0;
//};
//
//PixelShaderInput main(VertexShaderInput input)
//{
//	PixelShaderInput output;
//	float4 pos = float4(input.pos, 1.0f);
//
//	pos = mul(pos, model);
//	pos = mul(pos, view);
//	pos = mul(pos, projection);
//	output.pos = pos;
//
//	output.color = input.color;
//
//	return output;
//}