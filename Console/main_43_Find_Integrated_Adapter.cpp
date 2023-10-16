//#include <Windows.h>
//#include <functional>
//#include <Helpers/System.h>
//#include <Helpers/String.h>
//#include <iostream>
//#include <vector>
//#include <string>
//
//#include <wrl.h>
//#include <dxgi.h>
//#include <d3d11.h>
//#include <dxgi1_2.h>
//
//#pragma comment(lib, "D3D11.lib")
//#pragma comment(lib, "DXGI.lib")
//
//
//#ifdef _WIN32
//// Use Integrate Graphics by default.
//extern "C" {
//    // http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
//    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000000;
//
//    // http://developer.amd.com/community/blog/2015/10/02/amd-enduro-system-for-developers/
//    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 0;
//}
//#endif
//
//
//class EnumAdaptersState {
//public:
//    EnumAdaptersState()
//        : idx(0)
//    {
//        HRESULT hr = S_OK;
//        hr = CreateDXGIFactory1(IID_PPV_ARGS(this->dxgiFactory.GetAddressOf()));
//        H::System::ThrowIfFailed(hr);
//    }
//
//    Microsoft::WRL::ComPtr<IDXGIAdapter> Next() {
//        HRESULT hr = S_OK;
//        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
//
//        hr = this->dxgiFactory->EnumAdapters(this->idx++, adapter.GetAddressOf());
//        if (hr == DXGI_ERROR_NOT_FOUND) {
//            return nullptr;
//        }
//        H::System::ThrowIfFailed(hr);
//
//        return adapter;
//    }
//
//private:
//    uint32_t idx;
//    Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
//};
//
//
//class EnumOutputsState {
//public:
//    EnumOutputsState(Microsoft::WRL::ComPtr<IDXGIAdapter> adapter)
//        : idx(0)
//        , adapter(adapter)
//    {}
//
//    Microsoft::WRL::ComPtr<IDXGIOutput> Next() {
//        HRESULT hr = S_OK;
//        Microsoft::WRL::ComPtr<IDXGIOutput> output;
//
//        hr = this->adapter->EnumOutputs(this->idx++, output.GetAddressOf());
//        if (hr == DXGI_ERROR_NOT_FOUND) {
//            return nullptr;
//        }
//        H::System::ThrowIfFailed(hr);
//
//        return output;
//    }
//
//private:
//    uint32_t idx;
//    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
//};
//
//
//
//DXGI_ADAPTER_DESC1 GetAdapterDescription(const Microsoft::WRL::ComPtr<IDXGIAdapter>& dxgiAdapter) {
//    Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter1;
//    dxgiAdapter.As(&dxgiAdapter1);
//
//    DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
//    HRESULT hr = dxgiAdapter1->GetDesc1(&dxgiAdapterDesc);
//
//    if (SUCCEEDED(hr))
//        return dxgiAdapterDesc;
//
//    return {};
//}
//
//
//DXGI_OUTPUT_DESC GetOutputDescription(const Microsoft::WRL::ComPtr<IDXGIOutput>& dxgiOutput) {
//    DXGI_OUTPUT_DESC dxgiOutputDesc;
//    HRESULT hr = dxgiOutput->GetDesc(&dxgiOutputDesc);
//
//    if (SUCCEEDED(hr))
//        return dxgiOutputDesc;
//
//    return {};
//}
//
//
//bool FindAdapter(std::function<bool(Microsoft::WRL::ComPtr<IDXGIAdapter>)> fn) {
//    EnumAdaptersState state;
//
//    while (auto adapter = state.Next()) {
//        if (fn(std::move(adapter))) {
//            return true;
//        }
//    }
//    return false;
//}
//
//bool FindOutput(std::function<bool(Microsoft::WRL::ComPtr<IDXGIAdapter>, Microsoft::WRL::ComPtr<IDXGIOutput>)> fn) {
//    return FindAdapter([&](Microsoft::WRL::ComPtr<IDXGIAdapter> adapter) {
//        EnumOutputsState state(adapter);
//
//        while (auto output = state.Next()) {
//            if (fn(adapter, std::move(output))) {
//                return true;
//            }
//        }
//        return false;
//        });
//}
//
//void EnumAdapters(std::function<void(Microsoft::WRL::ComPtr<IDXGIAdapter>)> fn) {
//    FindAdapter([&](Microsoft::WRL::ComPtr<IDXGIAdapter> adapter) {
//        fn(std::move(adapter));
//        return false;
//        });
//}
//
//void EnumOutputs(std::function<void(Microsoft::WRL::ComPtr<IDXGIAdapter>, Microsoft::WRL::ComPtr<IDXGIOutput>)> fn) {
//    FindOutput([&](Microsoft::WRL::ComPtr<IDXGIAdapter> adapter, Microsoft::WRL::ComPtr<IDXGIOutput> output) {
//        fn(adapter, std::move(output));
//        return false;
//        });
//}
//
//
//Microsoft::WRL::ComPtr<IDXGIAdapter> FindCompatibleAdapterForDuplicateOutput() {
//    HRESULT hr = S_OK;
//
//    EnumAdaptersState enumAdapterState;
//    while (auto adapter = enumAdapterState.Next()) {
//
//        Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device;
//        Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11Context;
//
//        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
//        D3D_FEATURE_LEVEL m_featureLevel;
//        D3D_FEATURE_LEVEL featureLevels[] = {
//            D3D_FEATURE_LEVEL_11_1,
//            D3D_FEATURE_LEVEL_11_0,
//            D3D_FEATURE_LEVEL_10_1,
//            D3D_FEATURE_LEVEL_10_0,
//            D3D_FEATURE_LEVEL_9_3,
//            D3D_FEATURE_LEVEL_9_2,
//            D3D_FEATURE_LEVEL_9_1
//        };
//
//        //hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &d3d11Device, &m_featureLevel, &d3d11Context);
//        hr = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &d3d11Device, &m_featureLevel, &d3d11Context);
//        H::System::ThrowIfFailed(hr);
//
//        auto adapterDescription = GetAdapterDescription(adapter);
//        printf("Check adapter = %s ...\n", H::WStrToStr(adapterDescription.Description).c_str());
//
//        EnumOutputsState enumOutputState(adapter);
//        if (auto output = enumOutputState.Next()) {
//            Microsoft::WRL::ComPtr<IDXGIOutput1> output1;
//            hr = output->QueryInterface(output1.GetAddressOf());
//            H::System::ThrowIfFailed(hr);
//
//            Microsoft::WRL::ComPtr<IDXGIOutputDuplication> outputDuplicate;
//            hr = output1->DuplicateOutput(d3d11Device.Get(), outputDuplicate.GetAddressOf());
//            if (hr == E_ACCESSDENIED) {
//                //LOG_DEBUG_D("DuplicateOutput E_ACCESSDENIED"); // may occured if this process not running as SYSTEM and appears UAC (on secure desktop) 
//                Sleep(1);
//                continue;
//            }
//            else if (hr == DXGI_ERROR_UNSUPPORTED) {
//                //LOG_ERROR_D("DuplicateOutput DXGI_ERROR_UNSUPPORTED"); // may occured when NVIDIA set global graphics priority to use "High perfomance processor"
//                //LogLastError;
//                Sleep(1);
//                continue;
//            }
//            else if (FAILED(hr)) {
//                //LOG_ERROR_D("DuplicateOutput fail hr = [{:#08x}]", static_cast<unsigned int>(hr));
//                //LogLastError;
//                Sleep(1);
//                continue;
//            }
//
//            printf("Adapter found \n");
//            return adapter;
//        }
//    }
//
//    printf("Adapter not found \n");
//    return nullptr;
//}
//
//int main() {
//
//    printf("Enum adapters:\n");
//    int adapterCounter = 0;
//    EnumAdapters([&adapterCounter](Microsoft::WRL::ComPtr<IDXGIAdapter> adapter) {
//        auto adapterDescription = GetAdapterDescription(adapter);
//        printf("[%d] Adapter = %s\n", adapterCounter++, H::WStrToStr(adapterDescription.Description).c_str());
//        //printf("- description = %s\n", H::WStrToStr(adapterDescription.).c_str());
//        });
//
//
//    printf("\n");
//    printf("\n");
//    printf("Enum outputs:\n");
//    int outputCounter = 0;
//    EnumOutputs([&outputCounter](Microsoft::WRL::ComPtr<IDXGIAdapter> adapter, Microsoft::WRL::ComPtr<IDXGIOutput> output) {
//        auto adapterDescription = GetAdapterDescription(adapter);
//        auto outputDescription = GetOutputDescription(output);
//        RECT rect = outputDescription.DesktopCoordinates;
//        printf("[%d] Output [%s]\n", outputCounter++, H::WStrToStr(adapterDescription.Description).c_str());
//        printf(" - device name = %s\n", H::WStrToStr(outputDescription.DeviceName).c_str());
//        printf(" - desktop coords = {LT(%d, %d) RB(%d, %d)} [%dx%d]\n", rect.left, rect.top, rect.right, rect.bottom, (rect.right - rect.left), (rect.bottom - rect.top));
//        });
//
//    auto adapter = FindCompatibleAdapterForDuplicateOutput();
//    //auto description = GetAdapterDescription(adapter);
//    //printf("Adapter descript = %s\n", description.c_str());
//
//
//
//    //// Create DXGI factory:
//    //Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
//    //HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
//    //H::System::ThrowIfFailed(hr); // "Failed to create DXGI factory", "CreateDXGIFactory1"
//
//    //struct {
//    //    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
//    //    std::string description;
//    //} found;
//
//    //// Get a video adapter:
//    //Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter;
//    //for (UINT idxVideoAdapter = 0;
//    //    (hr = dxgiFactory->EnumAdapters1(idxVideoAdapter, dxgiAdapter.GetAddressOf())) != DXGI_ERROR_NOT_FOUND;
//    //    ++idxVideoAdapter)
//    //{
//    //    H::System::ThrowIfFailed(hr); // "Failed to enumerate video adapters", "IDXGIAdapter1::EnumAdapters1"
//
//    //    std::string description = GetAdapterDescription(dxgiAdapter);
//
//    //    printf("Adapter descript = %s\n", description.c_str());
//
//    //    if (idxVideoAdapter == 0) { // first adapter is the fallback
//    //        found = { dxgiAdapter, description };
//    //    }
//
//    //    //if (!gpuDeviceNameKey.empty()) {
//    //    //    if (H::to_lower(description).find(gpuDeviceNameKey) != std::string::npos) {
//    //    //        found = { dxgiAdapter, description };
//    //    //        break;
//    //    //    }
//    //    //}
//    //}
//
//
//    printf("\n");
//    Sleep(6000);
//	return 0;
//}
