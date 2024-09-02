
struct ConstData
{
    matrix mWorldViewProj;
};

cbuffer VS_ConstBufferA : register(b0)
{
    ConstData constDataA;
};

cbuffer VS_ConstBufferAA : register(b1)
{
    ConstData constDataAA;
};

export void VertexFunctionA(inout float4 rPosition, inout float2 rUV)
{
    rPosition = mul(rPosition, constDataA.mWorldViewProj);
}

export void VertexFunctionAA(inout float4 rPosition, inout float2 rUV)
{
    rPosition = mul(rPosition, constDataAA.mWorldViewProj);
}