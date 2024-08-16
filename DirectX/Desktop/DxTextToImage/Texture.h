#pragma once
#include "DxDevice.h"
#include <Helpers/Math.h>
#include <vector>
#include <string>

class Texture {
public:
    static std::vector<uint8_t> RenderTextToBytesArray(const std::wstring& text, H::Size texSize, int fontSize = 16);
    static std::vector<uint8_t> GetDataAsVector(ID2D1DeviceContext* d2dCtx);
};