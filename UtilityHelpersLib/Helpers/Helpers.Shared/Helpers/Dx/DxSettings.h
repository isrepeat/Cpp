#pragma once
#include "Helpers/common.h"
#include "Helpers/Event/Signal.h"
#include "DxHelpers.h"

#include <functional>
#include <vector>
#include <string>
#include <atomic>
#include <memory>

namespace HELPERS_NS {
    namespace Dx {
        class DxSettings {
        public:
			struct Events {
				HELPERS_NS::Event::Signal<void()> msaaChanged;
				HELPERS_NS::Event::Signal<void()> vsyncChanged;
				HELPERS_NS::Event::Signal<void()> hdrToneMappingSupportChanged;
				HELPERS_NS::Event::Signal<void()> adapersUpdated;
				HELPERS_NS::Event::Signal<void()> currentAdapterChanged;
			};

            DxSettings();

            const std::unique_ptr<Events>& GetEvents() const;

            void EnableMSAA(bool enabled);
            bool IsMSAAEnabled() const;

            void EnableVSync(bool enabled);
            bool IsVSyncEnabled() const;

            void EnableHDRToneMappingSupport(bool enabled);
            bool IsHDRToneMappingSupportEnabled() const;

            void SetCurrentAdapterByIdx(uint32_t idx);
            Adapter GetCurrentAdapter() const;

            void UpdateAdapters();
            std::vector<Adapter> GetAdapters() const;

        private:
            const std::unique_ptr<Events> events;
            std::atomic<bool> msaa;
            std::atomic<bool> vsync;
            std::atomic<bool> hdrToneMappingSupport;
            std::vector<Adapter> adapters;
            Adapter currentAdapter;
        };
    }
}