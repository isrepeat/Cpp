#pragma once
#include "Platform/IWma8CodecFactory.h"

class Wma8CodecFactory : public IWma8CodecFactory {
public:
    Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() override;
};