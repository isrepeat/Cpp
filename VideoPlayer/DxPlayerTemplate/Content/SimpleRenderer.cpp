#include "pch.h"
#include "SimpleRenderer.h"
#include <cmath>

using namespace DxPlayerTemplate;
using namespace DirectX;

SimpleRenderer::SimpleRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources)
	: m_deviceResources(deviceResources)
	, m_clearColor{ 0.2f, 0.2f, 0.35f, 1.0f }
{
	CreateDeviceDependentResources();
}

void SimpleRenderer::CreateDeviceDependentResources()
{
	// This renderer currently uses only the device back buffer.
}

void SimpleRenderer::CreateWindowSizeDependentResources()
{
	// No size-dependent resources are required for the clear-only renderer.
}

void SimpleRenderer::ReleaseDeviceDependentResources()
{
	// Nothing to release yet.
}

void SimpleRenderer::Update(const DX::StepTimer& timer)
{
	float time = static_cast<float>(timer.GetTotalSeconds());
	m_clearColor[0] = 0.25f + 0.25f * sinf(time * 0.5f);
	m_clearColor[1] = 0.25f + 0.25f * sinf(time * 0.7f + XM_PIDIV4);
	m_clearColor[2] = 0.35f + 0.25f * sinf(time * 0.9f + XM_PI);
}

void SimpleRenderer::Render()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();
	auto depthStencil = m_deviceResources->GetDepthStencilView();
	auto viewport = m_deviceResources->GetScreenViewport();

	context->RSSetViewports(1, &viewport);

	ID3D11RenderTargetView* const targets[] = { renderTarget };
	context->OMSetRenderTargets(1, targets, depthStencil);

	context->ClearRenderTargetView(renderTarget, m_clearColor);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
