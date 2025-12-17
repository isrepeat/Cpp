#pragma once
#include "D3dStructWrappers.h"
#include "MappedResource.h"

#include "State\OMRenderTargetState.h"
#include "State\RSViewportState.h"
#include "State\D3DTargetState.h"

#include <optional>

namespace DxHelpers {
	const DirectX::XMFLOAT2 standartDpi = DirectX::XMFLOAT2(96.0f, 96.0f);

	class MsaaHelper {
	public:
		static UINT GetMaxMSAA();
		static std::optional<DXGI_SAMPLE_DESC> GetMaxSupportedMSAA(ID3D11Device* d3dDev, DXGI_FORMAT format, UINT maxDesired);

	private:
		// Must be sorted from min to max
		// MSAA level 1 is not used as it's same as no MSAA
		// Theoretically D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT can be used but 2, 4, 8 msaa is the most common sample count in games
		static constexpr std::array<UINT, 3> MsaaLevels = { 2, 4, 8 };
	};
}
