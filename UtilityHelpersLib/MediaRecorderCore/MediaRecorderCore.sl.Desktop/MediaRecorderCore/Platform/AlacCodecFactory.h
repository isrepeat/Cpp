#pragma once
#include "Platform/IAlacCodecFactory.h"

class AlacCodecFactory : public IAlacCodecFactory {
public:
	Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() override;
};
