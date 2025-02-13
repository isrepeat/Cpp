// Структура входных данных для пиксельного шейдера
struct VSInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

// Структура выхода из вершинного шейдера
struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

// Вершинный шейдер: просто передает данные в пиксельный шейдер
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 1.0);
    output.texCoord = input.texCoord;
    return output;
}

// Пиксельный шейдер: рендерит градиент
float4 PSMain(PSInput input) : SV_TARGET
{
    //return float4(input.texCoord.x, input.texCoord.y, 0.5, 1.0);
    return float4(0.0, 0.0, 1.0, 1.0); // BGRA
}