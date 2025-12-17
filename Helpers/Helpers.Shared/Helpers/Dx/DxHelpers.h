#pragma once
#include <Helpers/common.h>
#include <Helpers/Rational.h>
#include "DxIncludes.h"
#include <optional>
#include <d3d11.h>
#include <dxgi.h>
#include <vector>
#include <string>
#include <cmath>
#include <wrl.h>

namespace HELPERS_NS {
    namespace Dx {
        namespace Colors {
            XMGLOBALCONST DirectX::XMVECTORF32 Background = { { { 0.254901975f, 0.254901975f, 0.254901975f, 1.f } } }; // #414141
            XMGLOBALCONST DirectX::XMVECTORF32 Green = { { { 0.062745102f, 0.486274511f, 0.062745102f, 1.f } } }; // #107c10
            XMGLOBALCONST DirectX::XMVECTORF32 Blue = { { { 0.019607844f, 0.372549027f, 0.803921580f, 1.f } } }; // #055fcd
            XMGLOBALCONST DirectX::XMVECTORF32 Orange = { { { 0.764705896f, 0.176470593f, 0.019607844f, 1.f } } }; // #c32d05
            XMGLOBALCONST DirectX::XMVECTORF32 DarkGrey = { { { 0.200000003f, 0.200000003f, 0.200000003f, 1.f } } }; // #333333
            XMGLOBALCONST DirectX::XMVECTORF32 LightGrey = { { { 0.478431374f, 0.478431374f, 0.478431374f, 1.f } } }; // #7a7a7a
            XMGLOBALCONST DirectX::XMVECTORF32 OffWhite = { { { 0.635294139f, 0.635294139f, 0.635294139f, 1.f } } }; // #a2a2a2
            XMGLOBALCONST DirectX::XMVECTORF32 White = { { { 0.980392158f, 0.980392158f, 0.980392158f, 1.f } } }; // #fafafa
        };

        namespace ColorsLinear {
            XMGLOBALCONST DirectX::XMVECTORF32 Background = { { { 0.052860655f, 0.052860655f, 0.052860655f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 Green = { { { 0.005181516f, 0.201556236f, 0.005181516f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 Blue = { { { 0.001517635f, 0.114435382f, 0.610495627f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 Orange = { { { 0.545724571f, 0.026241219f, 0.001517635f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 DarkGrey = { { { 0.033104762f, 0.033104762f, 0.033104762f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 LightGrey = { { { 0.194617808f, 0.194617808f, 0.194617808f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 OffWhite = { { { 0.361306787f, 0.361306787f, 0.361306787f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 White = { { { 0.955973506f, 0.955973506f, 0.955973506f, 1.f } } };
        };

        namespace ColorsHDR {
            XMGLOBALCONST DirectX::XMVECTORF32 Background = { { { 0.052860655f * 2.f, 0.052860655f * 2.f, 0.052860655f * 2.f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 Green = { { { 0.005181516f * 2.f, 0.201556236f * 2.f, 0.005181516f * 2.f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 Blue = { { { 0.001517635f * 2.f, 0.114435382f * 2.f, 0.610495627f * 2.f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 Orange = { { { 0.545724571f * 2.f, 0.026241219f * 2.f, 0.001517635f * 2.f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 DarkGrey = { { { 0.033104762f * 2.f, 0.033104762f * 2.f, 0.033104762f * 2.f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 LightGrey = { { { 0.194617808f * 2.f, 0.194617808f * 2.f, 0.194617808f * 2.f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 OffWhite = { { { 0.361306787f * 2.f, 0.361306787f * 2.f, 0.361306787f * 2.f, 1.f } } };
            XMGLOBALCONST DirectX::XMVECTORF32 White = { { { 0.955973506f * 2.f, 0.955973506f * 2.f, 0.955973506f * 2.f, 1.f } } };
        };



        class Adapter {
        private:
            friend class EnumAdaptersState;
            Adapter(Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter, uint32_t idx);
        public:
            Adapter();

            Microsoft::WRL::ComPtr<IDXGIAdapter> GetDXGIAdapter() const;
            uint32_t GetIndex() const;

            DXGI_ADAPTER_DESC1 GetDXGIDescription() const;
            std::wstring GetDescription() const;
            LUID GetAdapterLUID() const;

            operator bool() const {
                return static_cast<bool>(dxgiAdapter);
            }

        private:
            Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
            uint32_t idx;
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
        };


        class Output {
        private:
            friend class EnumOutputsState;
            Output(Microsoft::WRL::ComPtr<IDXGIOutput> output, uint32_t idx, Adapter adapter);
        public:
            Output();

            Microsoft::WRL::ComPtr<IDXGIOutput> GetDXGIOutput() const;
            uint32_t GetIndex() const;
            Adapter GetAdapter() const;
            
            DXGI_OUTPUT_DESC GetDXGIDescription() const;
            
            operator bool() const {
                return static_cast<bool>(dxgiOutput);
            }

        private:
            Microsoft::WRL::ComPtr<IDXGIOutput> dxgiOutput;
            uint32_t idx;
            Adapter adapter;
            DXGI_OUTPUT_DESC dxgiOutputDesc;
        };
        

        class EnumAdaptersState {
        public:
            EnumAdaptersState();
            Adapter Next();

        private:
            uint32_t idx;
            Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
        };


        class EnumOutputsState {
            friend Output;
        public:
            EnumOutputsState(Adapter adapter);
            Output Next();

        private:
            uint32_t idx;
            Adapter adapter;
        };



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



        void LogDeviceInfo();
        HELPERS_NS::Rational<double> GetRefreshRateForDXGIOutput(Microsoft::WRL::ComPtr<IDXGIOutput> dxgiOutput);

        // Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
        float ConvertDipsToPixels(float dips, float dpi);

#if defined(_DEBUG)
        // Check for SDK Layer support.
        bool SdkLayersAvailable();
#endif

        namespace Tools {
            std::vector<uint8_t> LoadBGRAImage(std::filesystem::path filename, uint32_t& width, uint32_t& height);
        }
    }
}