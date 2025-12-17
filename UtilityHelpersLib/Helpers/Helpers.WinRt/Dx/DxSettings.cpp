#include "pch.h"
#include "DxSettings.h"

namespace Helpers {
    namespace WinRt {
        namespace Dx {
            Adapter::Adapter(H::Dx::Adapter adapter)
                : adapter{ adapter }
            {}

            uint32_t Adapter::Idx::get() {
                return this->adapter.GetIndex();
            }
            Platform::String^ Adapter::Description::get() {
                return ref new Platform::String(this->adapter.GetDescription().c_str());
            }
            LUID Adapter::AdapterLUID::get() {
                LUID luid;
                static_assert(sizeof(luid.LowPart) == sizeof(adapter.GetAdapterLUID().LowPart));
                static_assert(sizeof(luid.HighPart) == sizeof(adapter.GetAdapterLUID().HighPart));

                luid.LowPart = this->adapter.GetAdapterLUID().LowPart;
                luid.HighPart = this->adapter.GetAdapterLUID().HighPart;
                return luid;
            }


            DxSettings::DxSettings()
                : dxSettings{ std::make_shared<H::Dx::DxSettings>() }
            {
                Platform::WeakReference weakRef = Platform::WeakReference(this);

                this->dxSettings->GetDxSettingsHandlers()->msaaChanged.Add([weakRef] {
                    auto _this = weakRef.Resolve<DxSettings>();
                    if (!_this) return;
                    concurrency::create_async([=]() {
                        _this->MsaaChanged();
                        });
                    });

                this->dxSettings->GetDxSettingsHandlers()->vsyncChanged.Add([weakRef] {
                    auto _this = weakRef.Resolve<DxSettings>();
                    if (!_this) return;
                    concurrency::create_async([=]() {
                        _this->VSyncChanged();
                        });
                    });

                this->dxSettings->GetDxSettingsHandlers()->currentAdapterChanged.Add([weakRef] {
                    auto _this = weakRef.Resolve<DxSettings>();
                    if (!_this) return;
                    concurrency::create_async([=]() {
                        _this->CurrentAdapterChanged();
                        });
                    });

                this->dxSettings->GetDxSettingsHandlers()->adapersUpdated.Add([weakRef] {
                    auto _this = weakRef.Resolve<DxSettings>();
                    if (!_this) return;
                    concurrency::create_async([=]() {
                        _this->AdaptersUpdated();
                        });
                    });
            }

            bool DxSettings::MSAA::get() {
                return this->dxSettings->IsMSAAEnabled();
            }
            void DxSettings::MSAA::set(bool enabled) {
                this->dxSettings->EnableMSAA(enabled);
            }

            bool DxSettings::VSync::get() {
                return this->dxSettings->IsVSyncEnabled();
            }
            void DxSettings::VSync::set(bool enabled) {
                this->dxSettings->EnableVSync(enabled);
            }

            bool DxSettings::HDRToneMappingSupport::get() {
                return this->dxSettings->IsHDRToneMappingSupportEnabled();
            }
            void DxSettings::HDRToneMappingSupport::set(bool enabled) {
                this->dxSettings->EnableHDRToneMappingSupport(enabled);
            }

            Adapter^ DxSettings::CurrentAdapter::get() {
                return ref new Adapter{ this->dxSettings->GetCurrentAdapter() };
            }
            void DxSettings::CurrentAdapter::set(Adapter^ adapter) {
                this->dxSettings->SetCurrentAdapterByIdx(adapter->Idx);
            }

            WFCollections::IObservableVector<Adapter^>^ DxSettings::Adapters::get() {
                auto result = ref new PCollections::Vector<Adapter^>;

                this->dxSettings->UpdateAdapters();
                for (auto& adapter : this->dxSettings->GetAdapters()) {
                    result->Append(ref new Adapter{ adapter });
                }
                return result;
            }

            std::shared_ptr<H::Dx::DxSettings> DxSettings::GetDxSettingsNative() {
                return this->dxSettings;
            }
        }
    }
}