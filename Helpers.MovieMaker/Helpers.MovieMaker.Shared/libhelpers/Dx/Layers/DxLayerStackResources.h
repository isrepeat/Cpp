#pragma once
#include "libhelpers\Dx\State\RSState.h"

#include <d3d11_1.h>
#include <wrl.h>

class DxLayerStackResources {
public:
	DxLayerStackResources(DxDevice* dxDeviceSafeObj);

	RSState SetToCtx(ID3D11DeviceContext *d3dCtx);

private:
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rsState;
};