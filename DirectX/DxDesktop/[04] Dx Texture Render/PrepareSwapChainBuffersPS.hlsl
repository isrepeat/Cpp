//--------------------------------------------------------------------------------------
// PrepareSwapChainBuffersPS.hlsl
//
// Takes the final HDR back buffer with linear values using Rec.709 color primaries and
// outputs the HDR signal. The HDR signal uses Rec.2020 color primaries
// with the ST.2084 curve.
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "HDRCommon.hlsli"

struct Interpolants
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

float4 currentNits : register(b0);     // Defines how bright white is (in nits), which controls how bright the SDR range in the image will be, e.g. 200 nits
Texture2D txDiffuse : register(t0);
SamplerState pointSampler : register(s0);


// Prepare the HDR swapchain buffer as HDR10. This means the buffer has to contain data which uses
//  - Rec.2020 color primaries
//  - Quantized using ST.2084 curve
//  - 10-bit per channel
float4 main(Interpolants In) : SV_Target0
{
    // Input is linear values using sRGB / Rec.709 color primaries
    float4 rgba = txDiffuse.Sample(pointSampler, In.texcoord);
    float3 rgb = (float3)rgba;

    //rgb = pow(rgb, 2.2);
    return float4(rgb, saturate(rgba.a));

    //return ConvertToHDR10(rgba, 1000);
    //return ConvertToHDR10_2(rgba, 10000);
    //return ConvertToHDR10_2(rgba, 1000);
}




//static const float3x3 ACESInputMat =
//{
//    { 0.59719, 0.35458, 0.04823 },
//    { 0.07600, 0.90834, 0.01566 },
//    { 0.02840, 0.13383, 0.83777 }
//};
//
//// ODT_SAT => XYZ => D60_2_D65 => sRGB
//static const float3x3 ACESOutputMat =
//{
//    { 1.60475, -0.53108, -0.07367 },
//    { -0.10208,  1.10813, -0.00605 },
//    { -0.00327, -0.07276,  1.07602 }
//};
//
//float3 RRTAndODTFit(float3 v)
//{
//    float3 a = v * (v + 0.0245786f) - 0.000090537f;
//    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
//    return a / b;
//}
//
//float4 main(Interpolants In) : SV_Target0
//{
//    //float4 color = D2DGetInput(0);
//    float4 color = txDiffuse.Sample(pointSampler, In.texcoord);
//    //color = float4(pow(color.rgb, 2.2), color.a);
//    float3 filmic = mul(ACESInputMat, color.rgb);
//
//    // Apply RRT and ODT
//    filmic = RRTAndODTFit(filmic);
//
//    filmic = mul(ACESOutputMat, filmic);
//
//    // Clamp to [0, 1]
//    filmic = saturate(filmic);
//
//    return float4(filmic, color.a);
//    //return float4(pow(filmic.rgb, 2.2), color.a);
//}