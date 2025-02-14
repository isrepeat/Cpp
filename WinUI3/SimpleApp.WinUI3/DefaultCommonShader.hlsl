
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


// Текстуры NV12 и Watermark
Texture2D inputY : register(t0); // Y-плоскость
Texture2D inputUV : register(t1); // UV-плоскость
Texture2D watermarkTexture : register(t2); // Watermark
SamplerState samplerState : register(s0);

// Константный буфер для позиции watermark
cbuffer WatermarkData : register(b0)
{
    float4 watermarkPos; // x, y, width, height в нормализованных координатах (0-1)
}

// Функция преобразования NV12 в BGRA
float4 ConvertNV12(float2 uv)
{
    float y = inputY.Sample(samplerState, uv).r;
    float2 uvSample = inputUV.Sample(samplerState, uv).rg;

    float u = uvSample.x - 0.5;
    float v = uvSample.y - 0.5;

    float r = y + 1.402 * v;
    float g = y - 0.344 * u - 0.714 * v;
    float b = y + 1.772 * u;

    return float4(r, g, b, 1.0);
}

// Проверяет, попадает ли пиксель в область watermark
bool IsInsideWatermark(float2 uv)
{
    return (uv.x >= watermarkPos.x && uv.x <= (watermarkPos.x + watermarkPos.z) &&
            uv.y >= watermarkPos.y && uv.y <= (watermarkPos.y + watermarkPos.w));
}

// Главный пиксельный шейдер
float4 PSMain(PSInput input) : SV_Target
{
    float4 frameColor = ConvertNV12(input.uv);

    // Применяем watermark только в указанной области
    if (IsInsideWatermark(input.uv))
    {
        float2 wmUV = (input.uv - watermarkPos.xy) / watermarkPos.zw;
        float4 watermarkColor = watermarkTexture.Sample(samplerState, wmUV);
        
        // Альфа-композитинг watermark поверх кадра
        return lerp(frameColor, watermarkColor, watermarkColor.a);
    }

    return frameColor;
}