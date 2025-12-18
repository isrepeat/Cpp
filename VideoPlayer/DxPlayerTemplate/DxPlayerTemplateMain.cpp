#include "pch.h"
#include "DxPlayerTemplateMain.h"

using namespace DxPlayerTemplate;

DxPlayerTemplateMain::DxPlayerTemplateMain(const std::shared_ptr<DX::DeviceResources>& deviceResources)
	: m_deviceResources(deviceResources)
{
	m_deviceResources->RegisterDeviceNotify(this);
	m_sceneRenderer = std::make_unique<SimpleRenderer>(m_deviceResources);
}

void DxPlayerTemplateMain::CreateWindowSizeDependentResources()
{
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

void DxPlayerTemplateMain::Update()
{
	m_timer.Tick([this]()
	{
		m_sceneRenderer->Update(m_timer);
	});
}

bool DxPlayerTemplateMain::Render()
{
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	m_sceneRenderer->Render();
	m_deviceResources->Present();
	return true;
}

void DxPlayerTemplateMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
}

void DxPlayerTemplateMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
