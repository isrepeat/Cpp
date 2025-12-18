#pragma once

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"
#include "Content/SimpleRenderer.h"

namespace DxPlayerTemplate
{
	class DxPlayerTemplateMain : public DX::IDeviceNotify
	{
	public:
		DxPlayerTemplateMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);

		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost() override;
		virtual void OnDeviceRestored() override;

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		std::unique_ptr<SimpleRenderer> m_sceneRenderer;
		DX::StepTimer m_timer;
	};
}
