#include "DxDeviceMt.h"

namespace Dx {
    namespace details {
        Microsoft::WRL::ComPtr<ID2D1Device> DxDeviceMt::GetD2DDevice() const {
            return d2dDevice;
        }
        Microsoft::WRL::ComPtr<ID2D1Factory1> DxDeviceMt::GetD2DFactory() const {
            return d2dFactory;
        }
        Microsoft::WRL::ComPtr<IDWriteFactory> DxDeviceMt::GetDwriteFactory() const {
            return dwriteFactory;
        }
        Microsoft::WRL::ComPtr<ID3D11Device> DxDeviceMt::GetD3DDevice() const {
            return d3dDevice;
        }
        Microsoft::WRL::ComPtr<ID3D10Multithread> DxDeviceMt::GetD3DMultithread() const {
            return d3dMultithread;
        }
    }
}