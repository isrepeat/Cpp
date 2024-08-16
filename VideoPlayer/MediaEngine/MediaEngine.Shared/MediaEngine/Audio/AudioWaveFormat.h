#pragma once
#include <algorithm>
#include <windows.h>
#include <cassert>
#include <mmreg.h>

class AudioWaveFormat {
public:
    AudioWaveFormat()
        : formatTag{ 0 }
        //, waveFormatEx{ 0 }
        , waveFormatExtensible{ WAVEFORMATEX{0} }
    {}

    AudioWaveFormat(WAVEFORMATEX* pWaveFormatEx)
        : formatTag{ pWaveFormatEx->wFormatTag }
        //, waveFormatEx{ 0 }
        , waveFormatExtensible{ WAVEFORMATEX{0} }
    {
        if (pWaveFormatEx->wFormatTag == WAVE_FORMAT_PCM) {
            //std::memcpy(&this->waveFormatEx, pWaveFormatEx, sizeof(this->waveFormatEx));
            std::memcpy(&this->waveFormatExtensible.Format, pWaveFormatEx, sizeof(WAVEFORMATEX));
        }
        else if (pWaveFormatEx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
            WAVEFORMATEXTENSIBLE* pWaveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pWaveFormatEx);
            std::memcpy(&this->waveFormatExtensible, pWaveFormatExtensible, sizeof(WAVEFORMATEXTENSIBLE));
        }
    }

    WORD GetFormatTag() const {
        return formatTag;
    }

    //WAVEFORMATEX GetWaveFormatEx() const {
    //    assert(formatTag == waveFormatEx.wFormatTag);
    //    return waveFormatEx;
    //}

    WAVEFORMATEXTENSIBLE GetWaveFormatExtensible() const {
        assert(formatTag == waveFormatExtensible.Format.wFormatTag);
        return waveFormatExtensible;
    }

    operator bool() const {
        return formatTag > 0;
    }

private:
    const WORD formatTag;
    //WAVEFORMATEX waveFormatEx;
    WAVEFORMATEXTENSIBLE waveFormatExtensible;
};