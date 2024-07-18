//--------------------------------------------------------------------------------------
// ToneMapSDRPS.hlsl
//
// Takes the final HDR back buffer with linear values using Rec.709 color primaries and
// outputs the SDR signal. The SDR signal uses Rec.709 color primaries which the
// hardware will apply the Rec.709 gamma curve.
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

struct Interpolants
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

Texture2D txDiffuse : register(t0);
SamplerState pointSampler : register(s0);

// Very simple HDR to SDR tonemapping, which simply clips values above 1.0f from the HDR scene.
float4 main(Interpolants In) : SV_Target0
{
    // Input is linear values using sRGB / Rec.709 color primaries
    float4 hdrSceneValues = txDiffuse.Sample(pointSampler, In.texcoord);

    return saturate(hdrSceneValues);
}