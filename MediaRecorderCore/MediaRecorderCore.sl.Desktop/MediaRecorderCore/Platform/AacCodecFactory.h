#pragma once
#include "Platform/IAacCodecFactory.h"

class AacCodecFactory : public IAacCodecFactory {
public:
	Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() override;
};
