
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
    // Загружаем основное изображение (inputTexture)
    float4 frameColor = inputTexture.Sample(samplerState, input.uv);

    // Применяем гамма-коррекцию
    frameColor.rgb = pow(frameColor.rgb, 2.2);

    if (IsInsideRegion(input.uv, watermarkPos))
    {
        float2 wmUV = (input.uv - watermarkPos.xy) / watermarkPos.zw;
        float4 watermarkColor = watermarkTexture.Sample(samplerState, wmUV);
        
        // Усиливаем желтоватый оттенок (смещаем баланс RGB)
        watermarkColor.r *= 1.3; // Усиливаем красный
        watermarkColor.g *= 1.05; // Усиливаем зелёный
        watermarkColor.b *= 0.95; // Немного ослабляем синий
        
        watermarkColor.rgb *= watermarkColor.a; // Предумножаем альфу
        
        if (watermarkColor.a > 0.2)
        {
            // Adding colors with different weights. You can experiment with weights.
            frameColor.rgb = frameColor.rgb * 1.7f + watermarkColor.rgb * 0.5f;
        }
    }
    
    // Применяем текст в указанной области
    if (IsInsideRegion(input.uv, textPos))
    {
        float2 txtUV = (input.uv - textPos.xy) / textPos.zw;
        float4 textColor = textTexture.Sample(samplerState, txtUV);

        // Гамма-коррекция текста
        textColor.rgb = pow(textColor.rgb, 2.2);

        // Альфа-композитинг текста поверх кадра
        frameColor.rgb = frameColor.rgb * (1 - textColor.a) + textColor.rgb * textColor.a;
    }

    // Обратная гамма-коррекция перед выводом
    frameColor.rgb = pow(frameColor.rgb, 1.0 / 2.2);

    return frameColor;
}
