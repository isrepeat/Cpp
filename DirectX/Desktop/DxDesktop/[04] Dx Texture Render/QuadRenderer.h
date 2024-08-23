#pragma once
#include <Helpers/Dx/DxIncludes.h>
#include "DxRenderObj.h"

namespace DxDesktop {
	class QuadRenderer {
	public:
		QuadRenderer(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj);
		~QuadRenderer() = default;

		void Draw(
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV,
			std::function<void __cdecl()> setCustomState = nullptr);
		
		void Draw(
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV,
			DxRenderObjBase* outterRenderObj,
			std::function<void __cdecl()> setCustomState);

	private:
		std::mutex mx;
		H::Dx::DxDeviceSafeObj* dxDeviceSafeObj;
		DxRenderObj defaultRenderObj;
	};
}