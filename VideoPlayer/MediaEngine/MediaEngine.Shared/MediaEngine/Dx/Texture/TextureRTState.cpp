#include "TextureRTState.h"

TextureRTState::TextureRTState(ID2D1DeviceContext *d2dCtx, ID3D11DeviceContext *d3dCtx)
    : D2DTargetState(d2dCtx), D3DTargetState(d3dCtx)
{}

TextureRTState::TextureRTState(TextureRTState &&other)
    : D2DTargetState(std::move(other)), D3DTargetState(std::move(other))
{}

TextureRTState::TextureRTState(D2DTargetState &&other1, D3DTargetState &&other2)
    : D2DTargetState(std::move(other1)), D3DTargetState(std::move(other2))
{}

TextureRTState::~TextureRTState() {}

TextureRTState &TextureRTState::operator=(TextureRTState &&other) {
    if (this != &other) {
        D2DTargetState::operator=(std::move(other));
        D3DTargetState::operator=(std::move(other));
    }

    return *this;
}