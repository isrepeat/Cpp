
struct ConstData
{
    matrix mWorldViewProj;
};

cbuffer ConstantBufferA : register(b0)
{
    ConstData constDataA;
};

cbuffer ConstantBufferB : register(b1)
{
    ConstData constDataB;
};

export void VertexFunctionA(inout float4 rPosition, inout float2 rUV)
{
    rPosition = mul(rPosition, constDataA.mWorldViewProj);
}

export void VertexFunctionB(inout float4 rPosition, inout float2 rUV)
{
    rPosition = mul(rPosition, constDataB.mWorldViewProj);
}