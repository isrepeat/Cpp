
struct ConstData
{
    matrix mWorldViewProj;
};

cbuffer VS_CONSTANT_BUFFER : register(b0)
{
    //matrix mWorldViewProj;
    ConstData constData;
};

//struct Vertex
//{
//    float4 position;
//    float2 texcoord;
//};

export void VertexFunctionA(inout float4 rPosition, inout float2 rUV)
{
    //rPosition = mul(rPosition, constData.mWorldViewProj);
    rPosition.x += 0.3;
}