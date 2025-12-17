#pragma once
#include "AvDxgiBufferFactory.h"

class AvDxgiBufferFactoryWin8 : public AvDxgiBufferFactory {
public:
    AvDxgiBufferFactoryWin8(DxDevice *dxDev)
        : AvDxgiBufferFactory(dxDev, &MFCreateDXGISurfaceBuffer, &MFCreateDXGIDeviceManager)
    {}

    virtual ~AvDxgiBufferFactoryWin8() {}
};