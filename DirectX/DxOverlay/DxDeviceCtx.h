#include <d3d11_1.h>
#include <d2d1_1.h>
#include <wrl.h>

class DxDeviceCtx {
public:
	DxDeviceCtx();
	DxDeviceCtx(
		const Microsoft::WRL::ComPtr<ID3D11DeviceContext1>& d3dCtx,
		const Microsoft::WRL::ComPtr<ID2D1DeviceContext>& d2dCtx);
	DxDeviceCtx(const DxDeviceCtx& other);
	DxDeviceCtx(DxDeviceCtx&& other);
	virtual ~DxDeviceCtx();

	DxDeviceCtx& operator=(const DxDeviceCtx& other);
	DxDeviceCtx& operator=(DxDeviceCtx&& other);

	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> D3D() const;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> D2D() const;

private:
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dCtx;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dCtx;
};