#pragma once
#include "D2DTargetState.h"
#include "D3DTargetState.h"
#include <Helpers/Macros.h>

class TextureRTState : public D2DTargetState, public D3DTargetState<1> {
public:
    NO_COPY(TextureRTState);

    TextureRTState(ID2D1DeviceContext *d2dCtx, ID3D11DeviceContext *d3dCtx);
    TextureRTState(TextureRTState &&other);
    TextureRTState(D2DTargetState &&other1, D3DTargetState &&other2);
    ~TextureRTState();

    TextureRTState &operator=(TextureRTState &&other);
};