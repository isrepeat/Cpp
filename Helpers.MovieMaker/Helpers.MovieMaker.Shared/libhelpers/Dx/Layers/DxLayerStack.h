#pragma once
#include "DxLayerStackResources.h"
#include "DxLayerStackState.h"
#include "..\DxDeviceCtxProvider.h"
#include "libhelpers\raw_ptr.h"
#include "libhelpers\PushStackScoped.h"

#include <d3d11_1.h>
#include <d2d1_1.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>

class DxLayerStack;

namespace DxLayerStackItems {
	struct DxLayerStackItemBase {
		DxLayerStackItemBase(DxLayerStack* dxLayerStack);
		~DxLayerStackItemBase() = default;
		NO_COPY(DxLayerStackItemBase);

	protected:
		DxLayerStack* dxLayerStack;
	};

	struct D2DLayer : DxLayerStackItemBase {
		D2DLayer(DxLayerStack* dxLayerStack);
		~D2DLayer() = default;

		void Push(const D2D1_LAYER_PARAMETERS& params, ID2D1Layer* layer = nullptr);
		void Pop();
	};

	struct RenderTarget : DxLayerStackItemBase {
		RenderTarget(DxLayerStack* dxLayerStack);
		~RenderTarget() = default;

		void Push();
		void Pop();
	};

	struct AxisAlignedClip : DxLayerStackItemBase {
		AxisAlignedClip(DxLayerStack* dxLayerStack);
		~AxisAlignedClip() = default;

		void Push(const D2D1_RECT_F& rect, D2D1_ANTIALIAS_MODE antialiasMode);
		void Pop();
	};
}

// TODO: add support move (for now if DxLayerStack was moved the pushed DxLayerStackItem was contain invalid DxLayerStack object)
// TODO: rewrite without 'friend' keyword workaround 
class DxLayerStack {
public:
	DxLayerStack(std::shared_ptr<DxDeviceCtxDynamic> dxCtxSafeObj, DxLayerStackResources* resources);
	~DxLayerStack() = default;

	NO_COPY_MOVE(DxLayerStack);

	// call before d3d draw calls to apply previous d2d layers to d3d rendering
	DxLayerStackState BeginD3D();

	PushStackScoped<DxLayerStackItems::D2DLayer> PushD2DLayerScoped(const D2D1_LAYER_PARAMETERS& params, ID2D1Layer* layer = nullptr);
	// call when start rendering to new D3D or/and D2D render target
	PushStackScoped<DxLayerStackItems::RenderTarget> PushRenderTargetScoped();
	PushStackScoped<DxLayerStackItems::AxisAlignedClip> PushAxisAlignedClipScoped(const D2D1_RECT_F& rect, D2D1_ANTIALIAS_MODE antialiasMode);

private:
	static D2D1_RECT_F ConcatRects(
		D2D1_RECT_F existing,
		D2D1_RECT_F other,
		D2D1_ANTIALIAS_MODE antialiasMode,
		const D2D1_MATRIX_3X2_F& transform);

	static D2D1_RECT_F ConcatRects(
		const D2D1_RECT_F& existing,
		const D2D1_RECT_F& other,
		D2D1_ANTIALIAS_MODE antialiasMode);

	static bool IsRectInfinite(const D2D1_RECT_F& v);

	D2D1_RECT_F GetCurrentRect();

private:
	std::shared_ptr<DxDeviceCtxDynamic> dxCtxSafeObj;
	DxLayerStackResources* resources;

	std::vector<D2D1_RECT_F> layerSizes;
	std::vector<std::vector<D2D1_RECT_F>> layerSizesStack;

	friend DxLayerStackItems::D2DLayer;
	DxLayerStackItems::D2DLayer dxLayerD2DLayer;

	friend DxLayerStackItems::RenderTarget;
	DxLayerStackItems::RenderTarget dxLayerRenderTarget;

	friend DxLayerStackItems::AxisAlignedClip;
	DxLayerStackItems::AxisAlignedClip dxLayerAxisAlignedClip;
};