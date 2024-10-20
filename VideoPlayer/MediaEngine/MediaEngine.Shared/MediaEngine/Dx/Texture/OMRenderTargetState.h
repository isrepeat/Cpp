#pragma once
#include <Helpers/Dx/DxDevice.h>
#include "comptr_array.h"
#include <Helpers/Macros.h>

template<uint32_t RtCount>
class OMRenderTargetState {
public:
	NO_COPY(OMRenderTargetState);

	OMRenderTargetState(ID3D11DeviceContext *d3dCtx)
		:d3dCtx(d3dCtx)
	{
		this->d3dCtx->OMGetRenderTargets(RtCount, this->rtv.data(), this->dsv.GetAddressOf());
	}

	OMRenderTargetState(OMRenderTargetState &&other)
		: d3dCtx(std::move(other.d3dCtx)),
		rtv(std::move(other.rtv)),
		dsv(std::move(other.dsv))
	{
		other.d3dCtx = nullptr;
	}

	~OMRenderTargetState() {
		if (this->d3dCtx) {
			this->d3dCtx->OMSetRenderTargets(RtCount, this->rtv.data(), this->dsv.Get());
		}
	}

	OMRenderTargetState &operator=(OMRenderTargetState &&other) {
		if (this != &other) {
			this->d3dCtx = std::move(other.d3dCtx);
			this->rtv = std::move(other.rtv);
			this->dsv = std::move(other.dsv);

			other.d3dCtx = nullptr;
		}

		return *this;
	}

private:
	ID3D11DeviceContext *d3dCtx;

	comptr_array<ID3D11RenderTargetView, RtCount> rtv;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;
};