#pragma once
#include "Platform/IFlacCodecFactory.h"

class FlacCodecFactory : public IFlacCodecFactory {
public:
	Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() override;
};
