#pragma once
#include "Platform/IMP3CodecFactory.h"

class MP3CodecFactory : public IMP3CodecFactory {
public:
    Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() override;
};