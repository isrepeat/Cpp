#include "pch.h"
#include "DxHelpers.h"
#include "..\HSystem.h"

#include <cassert>

namespace DxHelpers {
    UINT MsaaHelper::GetMaxMSAA() {
        return *(std::end(MsaaHelper::MsaaLevels) - 1);
    }

    std::optional<DXGI_SAMPLE_DESC> MsaaHelper::GetMaxSupportedMSAA(ID3D11Device* d3dDev, DXGI_FORMAT format, UINT maxDesired) {
        if (!d3dDev) {
            assert(false);
            return {};
        }

        // find closest default msaa level
        auto it = std::upper_bound(std::begin(MsaaHelper::MsaaLevels), std::end(MsaaHelper::MsaaLevels) - 1, maxDesired);
        assert(it != std::end(MsaaHelper::MsaaLevels));

        DXGI_SAMPLE_DESC sampleDesc = {};
        sampleDesc.Count = *it;
        HRESULT hr = E_FAIL;

        // decrease msaa level if selected unsupported
        while (FAILED(hr)) {
            sampleDesc.Quality = -1;
            hr = d3dDev->CheckMultisampleQualityLevels(format, sampleDesc.Count, &sampleDesc.Quality);

            if (sampleDesc.Quality == 0 && SUCCEEDED(hr)) {
                hr = E_FAIL;
            }
            assert(sampleDesc.Quality > 0);

            // CheckMultisampleQualityLevels returns count of quality levels, select max level
            --sampleDesc.Quality;

            if (it == std::begin(MsaaHelper::MsaaLevels)) {
                break;
            }
            --it;
        }

        if (FAILED(hr)) {
            return {};
        }

        return sampleDesc;
    }
}
