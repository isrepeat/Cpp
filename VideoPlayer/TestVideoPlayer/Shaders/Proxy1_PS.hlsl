struct Interpolants
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

Texture2D txDiffuse : register(t0);
SamplerState pointSampler : register(s0);

float4 main(Interpolants In) : SV_Target
{
    float4 color = txDiffuse.Sample(pointSampler, In.texcoord);
    return color;
    //return float4(1, 0, 0, 1);
}