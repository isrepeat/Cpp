
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


Texture2D inputTexture : register(t0);
Texture2D watermarkTexture : register(t1);
Texture2D textTexture : register(t2);

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


// Проверяет, находится ли пиксель в области watermark
bool IsInsideRegion(float2 uv, float4 region)
{
    return (uv.x >= region.x && uv.x <= (region.x + region.z) &&
            uv.y >= region.y && uv.y <= (region.y + region.w));
}

// Основной пиксельный шейдер
float4 PSMain(PSInput input) : SV_Target
{
    float4 frameColor = inputTexture.Sample(samplerState, input.uv);

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