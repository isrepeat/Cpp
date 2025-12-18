#pragma once

#include "Common/DeviceResources.h"
#include "Common/StepTimer.h"

namespace DxPlayerTemplate
{
	class SimpleRenderer
	{
	public:
		SimpleRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);

		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(const DX::StepTimer& timer);
		void Render();

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		float m_clearColor[4];
	};
}
