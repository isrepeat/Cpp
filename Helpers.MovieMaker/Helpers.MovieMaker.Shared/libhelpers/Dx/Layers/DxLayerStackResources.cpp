#include "pch.h"
#include "DxLayerStackResources.h"
#include "libhelpers\HSystem.h"

DxLayerStackResources::DxLayerStackResources(DxDevice* dxDeviceSafeObj) {
	HRESULT hr = S_OK;
	D3D11_RASTERIZER_DESC rsDesc = {};

	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = FALSE;
	rsDesc.DepthBias = 0;
	rsDesc.SlopeScaledDepthBias = 0.0f;
	rsDesc.DepthBiasClamp = 0.0f;
	rsDesc.DepthClipEnable = TRUE;
	rsDesc.ScissorEnable = TRUE;
	rsDesc.MultisampleEnable = FALSE;
	rsDesc.AntialiasedLineEnable = FALSE;

	auto dxDev = dxDeviceSafeObj->Lock();
	auto d3dDev = dxDev->GetD3DDevice();
	hr = d3dDev->CreateRasterizerState(&rsDesc, this->rsState.GetAddressOf());
	H::System::ThrowIfFailed(hr);
}

RSState DxLayerStackResources::SetToCtx(ID3D11DeviceContext *d3dCtx) {
	RSState state(d3dCtx);

	d3dCtx->RSSetState(this->rsState.Get());

	return state;
}