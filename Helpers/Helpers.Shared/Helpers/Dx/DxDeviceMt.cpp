#include "DxDeviceMt.h"

namespace HELPERS_NS {
    namespace Dx {
        namespace details {
            Microsoft::WRL::ComPtr<ID2D1Device> DxDeviceMt::GetD2DDevice() const {
                return this->d2dDevice;
            }
            Microsoft::WRL::ComPtr<ID2D1Factory3> DxDeviceMt::GetD2DFactory() const {
                return this->d2dFactory;
            }
            Microsoft::WRL::ComPtr<IDWriteFactory3> DxDeviceMt::GetDwriteFactory() const {
                return this->dwriteFactory;
            }
            Microsoft::WRL::ComPtr<IWICImagingFactory2> DxDeviceMt::GetWICFactory() const {
                return this->wicFactory;
            }
            Microsoft::WRL::ComPtr<IDXGIFactory2> DxDeviceMt::GetDxgiFactory() const {
                return this->dxgiFactory;
            }
            Microsoft::WRL::ComPtr<ID3D11Device3> DxDeviceMt::GetD3DDevice() const {
                return this->d3dDevice;
            }
            Microsoft::WRL::ComPtr<ID3D10Multithread> DxDeviceMt::GetD3DMultithread() const {
                return this->d3dMultithread;
            }

            HELPERS_NS::Dx::D2DCtxMt* DxDeviceMt::GetD2DCtxMt() {
                return &this->d2dCtxMt;
            }
        }
    }
}