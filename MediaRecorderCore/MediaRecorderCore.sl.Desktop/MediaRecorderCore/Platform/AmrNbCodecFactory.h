#pragma once
#include "Platform/IAmrNbCodecFactory.h"

class AmrNbCodecFactory : public IAmrNbCodecFactory {
public:
    Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() override;
};