#include "NoiseFilter.hlsli"

Texture2D Tex : register(t0);

cbuffer Buf0 : register(b0){
	float GlobalTime;
	float Opacity;
	matrix ColorEffect;
}

SamplerState TexSampler : register(s0);

struct PsInput{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PsInput input) : SV_TARGET{
	float4 color = Tex.Sample(TexSampler, input.tex);
    //return color;
	color = mul(color, ColorEffect);
	color *= Opacity;

	return ApplyNoise(color, input.tex, GlobalTime);
}