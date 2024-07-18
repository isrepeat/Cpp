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
    float4 hdrSceneValues = txDiffuse.Sample(pointSampler, In.texcoord);

    return ConvertToHDR10(hdrSceneValues, 1000);
}


//
//SamplerState PointSampler : register(s0);
//Texture2D<float4> Texture : register(t0);
//
//struct Interpolators
//{
//    float4 Position : SV_Position;
//    float2 TexCoord : TEXCOORD0;
//};
//
//float4 PaperWhiteNits           : register(b0);     // Defines how bright white is (in nits), which controls how bright the SDR range in the image will be, e.g. 200 nits
//
//// Prepare the HDR swapchain buffer as HDR10. This means the buffer has to contain data which uses
////  - Rec.2020 color primaries
////  - Quantized using ST.2084 curve
////  - 10-bit per channel
//float4 main(Interpolators In) : SV_Target0
//{
//    // Input is linear values using sRGB / Rec.709 color primaries
//    float4 hdrSceneValues = Texture.Sample(PointSampler, In.TexCoord);
//
//    //float4 RedColor = { 0.2f, 0.0f, 0.0f, 1.0f };
//    //return RedColor;
//
//    //return ConvertToHDR10(hdrSceneValues, PaperWhiteNits.x);
//    return ConvertToHDR10(hdrSceneValues, 1000);
//}