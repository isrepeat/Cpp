
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


// Текстуры NV12, Watermark и Text Overlay
Texture2D inputY : register(t0); // Y-плоскость
Texture2D inputUV : register(t1); // UV-плоскость
Texture2D watermarkTexture : register(t2); // Watermark
Texture2D textTexture : register(t3); // Текст

SamplerState samplerState : register(s0);

// Константный буфер для позиции watermark
cbuffer WatermarkData : register(b0)
{
    float4 watermarkPos; // x, y, width, height в нормализованных координатах (0-1)
}

// Константный буфер для позиции текста
cbuffer TextData : register(b1)
{
    float4 textPos; // x, y, width, height в нормализованных координатах (0-1)
}

// Функция конвертации NV12 в BGRA
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

// Проверяет, находится ли пиксель в области watermark
bool IsInsideRegion(float2 uv, float4 region)
{
    return (uv.x >= region.x && uv.x <= (region.x + region.z) &&
            uv.y >= region.y && uv.y <= (region.y + region.w));
}

// Основной пиксельный шейдер
float4 PSMain(PSInput input) : SV_Target
{
    float4 frameColor = ConvertNV12(input.uv);

    // Применяем watermark в указанной области
    if (IsInsideRegion(input.uv, watermarkPos))
    {
        float2 wmUV = (input.uv - watermarkPos.xy) / watermarkPos.zw;
        float4 watermarkColor = watermarkTexture.Sample(samplerState, wmUV);
        
        // Альфа-композитинг watermark поверх кадра
        frameColor = lerp(frameColor, watermarkColor, watermarkColor.a);
    }

    // Применяем текст в указанной области
    if (IsInsideRegion(input.uv, textPos))
    {
        float2 txtUV = (input.uv - textPos.xy) / textPos.zw;
        float4 textColor = textTexture.Sample(samplerState, txtUV);

        // Альфа-композитинг текста поверх кадра
        frameColor = lerp(frameColor, textColor, textColor.a);
    }

    return frameColor;
}