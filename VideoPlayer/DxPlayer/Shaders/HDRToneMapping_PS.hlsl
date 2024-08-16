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


// https://github.com/thefranke/dirtchamber/blob/master/shader/tonemapping.hlsl
#define TONEMAP_GAMMA 1.0
//#define TONEMAP_GAMMA 1.0 / 2.2

// Reinhard Tonemapper
float4 tonemap_reinhard(in float3 color)
{
    color *= 16;
    color = color / (1 + color);
    float3 ret = pow(color, TONEMAP_GAMMA); // gamma
    return float4(ret, 1);
}

// Uncharted 2 Tonemapper
float3 tonemap_uncharted2(in float3 x)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 tonemap_uc2(in float3 color)
{
    float W = 11.2;

    color *= 16; // Hardcoded Exposure Adjustment
    //float exposure_bias = 100.0f;
    float exposure_bias = 2.0f;

    float3 curr = tonemap_uncharted2(exposure_bias * color);

    float3 white_scale = 1.0f / tonemap_uncharted2(W);
    float3 ccolor = curr * white_scale;

    float3 ret = pow(abs(ccolor), TONEMAP_GAMMA); // gamma
    return ret;
}

// Filmic tonemapper
float3 tonemap_filmic(in float3 color)
{
    color = max(0, color - 0.004f);
    color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f) + 0.06f);

    // result has 1/2.2 baked in
    return pow(color, TONEMAP_GAMMA);
}




struct Interpolants
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

Texture2D txDiffuse : register(t0);
SamplerState pointSampler : register(s0);


//// Very simple HDR to SDR tonemapping, which simply clips values above 1.0f from the HDR scene.
//float4 main(Interpolants In) : SV_Target0
//{
//    // Input is linear values using sRGB / Rec.709 color primaries
//    float4 pixColor = txDiffuse.Sample(pointSampler, In.texcoord); // hdr values
//
//    //float3 res = tonemap_reinhard(pixColor.rgb);
//    //float3 res = tonemap_uncharted2(pixColor.rgb);
//    float3 res = tonemap_uc2(pixColor.rgb); // <-- Good
//    //float3 res = tonemap_filmic(pixColor.rgb);
//
//    float4 output = float4(res.rgb, pixColor.a);
//    return output;
//
//    //return saturate(pixColor);
//}


// aka tonemap_uncharted2
inline float hable(float x)
{
    const float A = 0.15, B = 0.50, C = 0.10, D = 0.20, E = 0.02, F = 0.30;
    return ((x * (A * x + (C * B)) + (D * E)) / (x * (A * x + B) + (D * F))) - E / F;
}

inline float3 hable(float3 x)
{
    x.r = hable(x.r);
    x.g = hable(x.g);
    x.b = hable(x.b);
    return x;
}

inline float3 sourceToLinear(float3 rgb)
{
    const float ST2084_m1 = 2610.0 / (4096.0 * 4);
    const float ST2084_m2 = (2523.0 / 4096.0) * 128.0;
    const float ST2084_c1 = 3424.0 / 4096.0;
    const float ST2084_c2 = (2413.0 / 4096.0) * 32.0;
    const float ST2084_c3 = (2392.0 / 4096.0) * 32.0;

    rgb = pow(rgb, 1.0 / ST2084_m2);
    rgb = max(rgb - ST2084_c1, 0.0) / (ST2084_c2 - ST2084_c3 * rgb);
    rgb = pow(rgb, 1.0 / ST2084_m1);
    return rgb;
}

inline float3 toneMapping(float3 rgb)
{
    const float LuminanceScale = 100.0f; // Max full frame luminance? // TODO: must be set from constant buffer
    static const float3 HABLE_DIV = hable(11.2);
    rgb = hable(rgb * LuminanceScale) / HABLE_DIV;
    return rgb;
}

inline float3 linearToDisplay(float3 rgb)
{
    float toneMapGamma = 1.0 / 2.2;
    return pow(rgb, toneMapGamma);
}


inline float3 adjustRange(float3 rgb)
{
    return rgb;
}

float4 main(Interpolants In) : SV_Target0
{
    float4 rgba = txDiffuse.Sample(pointSampler, In.texcoord);

    const float Opacity = 1.0f; // TODO: must be set from constant buffer
    float opacity = rgba.a * Opacity;
    float3 rgb = (float3)rgba;

    
    //return float4(rgb, 1.0);
    //return float4(-999999, -999999, -999999, 1.0f);
    
    rgb = float3(0.0, 0.0, 0.0);
    //return float4(pow(rgb, 1.0 / 2.2), 1.0);
    
    
    //rgb = pow(rgb, 2.2);
    
    //return float4(1.0, 0.0, 0.0, 1.0);
    //return float4(pow(rgba.rgb, 1.0 / 2.2), rgba.a);
    //return float4(pow(rgba.rgb, 2.2), rgba.a);

    //return float4(rgb, saturate(opacity));

    rgb = sourceToLinear(rgb);
    rgb = toneMapping(rgb);
    return float4(pow(rgb, 1.0 / 2.2), 1.0);
    // TODO: Check in cpp what value rgb is
    
    //return float4(rgb, 1.0);
    rgb = linearToDisplay(rgb);
    rgb = adjustRange(rgb);

    //rgb = tonemap_uc2(rgb);

    //rgba = pow(rgb, 1.0 / 2.2);
    //rgb = pow(rgb, 2.2);
    
    //return float4(rgb, saturate(opacity));
    //return float4(rgb, 1.0);
}