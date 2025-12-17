#pragma once
#include <Helpers/Dx/DxSettings.h>
#include <Helpers/Event/IEvent.h>
#include "Delegates.h"
#include <vector>

namespace Helpers {
    namespace WinRt {
        namespace Dx {
            public value struct LUID {
                uint32_t LowPart;
                int32_t HighPart;
            };

            public ref class Adapter sealed {
            internal:
                Adapter(H::Dx::Adapter adapter);

            public:
                property uint32_t Idx {
                    uint32_t get();
                }
                property Platform::String^ Description {
                    Platform::String^ get();
                }
                property LUID AdapterLUID {
                    LUID get();
                }

            internal:
                const H::Dx::Adapter adapter;
            };


            public ref class DxSettings sealed {
            public:
                DxSettings();

                event EventHandler^ MsaaChanged;
                event EventHandler^ VSyncChanged;
                event EventHandler^ HDRToneMappingSupportChanged;
                event EventHandler^ CurrentAdapterChanged;
                event EventHandler^ AdaptersUpdated;

                property bool MSAA {
                    bool get();
                    void set(bool enabled);
                }
                property bool VSync {
                    bool get();
                    void set(bool enabled);
                }
                property bool HDRToneMappingSupport {
                    bool get();
                    void set(bool enabled);
                }

                property Adapter^ CurrentAdapter {
                    Adapter^ get();
                    void set(Adapter^ adapter);
                }
                property WFCollections::IObservableVector<Adapter^>^ Adapters {
                    WFCollections::IObservableVector<Adapter^>^ get();
                }

            internal:
                std::shared_ptr<H::Dx::DxSettings> GetDxSettingsNative();

            private:
                std::shared_ptr<H::Dx::DxSettings> dxSettings;
                std::vector<std::shared_ptr<H::Event::ISubscriptionToken>> eventSubscriptions;
            };
        }
    }
}
