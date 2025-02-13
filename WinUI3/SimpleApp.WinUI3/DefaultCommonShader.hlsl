// Структура входных данных для пиксельного шейдера
struct VSInput
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

// Структура выхода из вершинного шейдера
struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

// Вершинный шейдер: просто передает данные в пиксельный шейдер
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.pos = float4(input.position, 1.0);
    output.uv = input.texCoord;
    return output;
}



// Текстура Y и UV (NV12 формат)
Texture2D inputY : register(t0); // Y-плоскость
Texture2D inputUV : register(t1); // UV-плоскость
SamplerState samplerState : register(s0);


float4 PSMain(PSInput input) : SV_Target
{
    float y = inputY.Sample(samplerState, input.uv).r;
    float2 uv = inputUV.Sample(samplerState, input.uv).rg;

    float u = uv.x - 0.5;
    float v = uv.y - 0.5;

    float r = y + 1.402 * v;
    float g = y - 0.344 * u - 0.714 * v;
    float b = y + 1.772 * u;

    return float4(r, g, b, 1.0);
}

//// Пиксельный шейдер: рендерит градиент
//float4 PSMain(PSInput input) : SV_TARGET
//{
//    //return float4(input.texCoord.x, input.texCoord.y, 0.5, 1.0);
//    return float4(0.0, 0.0, 1.0, 1.0); // BGRA
//}


//// Преобразование YUV в RGB
//float3 YUVtoRGB(float y, float u, float v)
//{
//    u = u - 0.5;
//    v = v - 0.5;

//    float r = y + 1.402 * v;
//    float g = y - 0.344 * u - 0.714 * v;
//    float b = y + 1.772 * u;

//    return float3(r, g, b);
//}

//// Пиксельный шейдер
//float4 PSMain(PSInput input) : SV_TARGET
//{
//    // Получаем Y значение (прямо из Y-плоскости)
//    float y = inputY.Sample(samplerState, input.uv).r;

//    // ✅ Берем координаты UV и правильно масштабируем их
//    float2 uvCoord = floor(input.uv * float2(textureWidth / 2, textureHeight / 2)) / float2(textureWidth / 2, textureHeight / 2);
//    float2 uv = inputUV.Sample(samplerState, uvCoord).rg;

//    float3 rgb = YUVtoRGB(y, uv.x, uv.y);
//    return float4(rgb, 1.0);
//}