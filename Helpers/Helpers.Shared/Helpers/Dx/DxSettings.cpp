#include "DxSettings.h"
#include <functional>
#include <vector>
#include <string>
#include <atomic>

namespace HELPERS_NS {
    namespace Dx {
		DxSettings::DxSettings()
			: events{ std::make_unique<Events>() }
			, msaa{ false }
			, vsync{ true }
			, hdrToneMappingSupport{ true }
		{
			this->UpdateAdapters();
			this->currentAdapter = adapters.at(0);
		}


		const std::unique_ptr<DxSettings::Events>& DxSettings::GetEvents() const {
			return this->events;
		}


        void DxSettings::EnableMSAA(bool enabled) {
            this->msaa = enabled;
			this->events->msaaChanged.Invoke();
        }

        bool DxSettings::IsMSAAEnabled() const {
            return this->msaa;
        }


        void DxSettings::EnableVSync(bool enabled) {
            this->vsync = enabled;
			this->events->vsyncChanged.Invoke();
        }

        bool DxSettings::IsVSyncEnabled() const {
            return this->vsync;
        }


        void DxSettings::EnableHDRToneMappingSupport(bool enabled) {
            this->hdrToneMappingSupport = enabled;
			this->events->hdrToneMappingSupportChanged.Invoke();
        }

        bool DxSettings::IsHDRToneMappingSupportEnabled() const {
            return this->hdrToneMappingSupport;
        }


        void DxSettings::SetCurrentAdapterByIdx(uint32_t idx) {
            if (idx >= this->adapters.size()) {
                return;
            }
            this->currentAdapter = this->adapters[idx];
			this->events->currentAdapterChanged.Invoke();
        }

        Adapter DxSettings::GetCurrentAdapter() const {
            return this->currentAdapter;
        }

 
        void DxSettings::UpdateAdapters() {
            // TODO: add mutex
            this->adapters.clear();
            EnumAdaptersState enumAdapters;

            while (auto adapter = enumAdapters.Next()) {
                this->adapters.push_back(adapter);
            }
			this->events->adapersUpdated.Invoke();
        }

        std::vector<Adapter> DxSettings::GetAdapters() const {
            return this->adapters;
        }
    }
}