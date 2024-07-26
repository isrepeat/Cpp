#include "SimpleSceneRenderer.h"
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/Helpers.h>

#include "Tools.h"
#include <DirectXMath.h>
#include <fstream>

#pragma comment(lib, "dxguid.lib")

namespace DxDesktop {
	SimpleSceneRenderer::SimpleSceneRenderer(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel)
		: swapChainPanel{ swapChainPanel }
		, quadRenderer{ swapChainPanel->GetDxDevice() }
		, keyboard{ std::make_unique<DirectX::Keyboard>() }
		, currentNits{ 1500.0f }
	{
		HRESULT hr = S_OK;

		this->CreateDeviceDependentResources();
		this->CreateWindowSizeDependentResources();
	}

	SimpleSceneRenderer::~SimpleSceneRenderer()
	{}

	// Executes basic render loop.
	void SimpleSceneRenderer::Tick() {
		this->renderLoopTimer.Tick([this] {
			this->Update();
		});

		this->Render();
	}

	void SimpleSceneRenderer::OnWindowSizeChanged(H::Size newSize) {
		this->swapChainPanel->SetLogicalSize(static_cast<H::Size_f>(newSize));
		this->CreateWindowSizeDependentResources();
	}


	void SimpleSceneRenderer::CreateDeviceDependentResources() {
		HRESULT hr = S_OK;

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();
		
		this->dxRenderObjImage = std::make_unique<DxRenderObjImage>(this->swapChainPanel);
		this->dxRenderObjProxy = std::make_unique<DxRenderObjProxy>(this->swapChainPanel);
		this->dxRenderObjHdrQuad = std::make_unique<DxRenderObjHdrQuad>(this->swapChainPanel);
	}

	void SimpleSceneRenderer::CreateWindowSizeDependentResources() {
		this->dxRenderObjImage->CreateWindowSizeDependentResources();
		this->dxRenderObjProxy->CreateWindowSizeDependentResources();
		this->dxRenderObjHdrQuad->CreateWindowSizeDependentResources();
	}

	void SimpleSceneRenderer::ReleaseDeviceDependentResources() {
		this->dxRenderObjImage->ReleaseDeviceDependentResources();
		this->dxRenderObjProxy->ReleaseDeviceDependentResources();
		this->dxRenderObjHdrQuad->ReleaseDeviceDependentResources();
	}

	void SimpleSceneRenderer::Update() {
		using DirectX::Keyboard;

		auto kb = this->keyboard->GetState();
		this->keyboardButtons.Update(kb);

		bool nitsChanged = false;

		if (kb.Escape) {
			//ExitSample();
		}

		if (this->keyboardButtons.IsKeyPressed(Keyboard::Space)) {
		}

		if (this->keyboardButtons.IsKeyPressed(Keyboard::OemMinus) || this->keyboardButtons.IsKeyPressed(Keyboard::Subtract)) {
			nitsChanged = true;
			this->currentNits -= 200.0f;
			this->currentNits = std::max(this->currentNits, 200.0f);
		}

		if (this->keyboardButtons.IsKeyPressed(Keyboard::OemPlus) || this->keyboardButtons.IsKeyPressed(Keyboard::Add)) {
			nitsChanged = true;
			this->currentNits += 200.0f;
			this->currentNits = std::max(this->currentNits, DX::c_MaxNitsFor2084);
		}

		if (kb.Down || kb.Left) {
		}

		if (kb.Up || kb.Right){
		}

		//// Update constant buffer
		//if (nitsChanged) {
		//	auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		//	auto dxCtx = dxDev->LockContext();
		//	auto d3dCtx = dxCtx->D3D();

		//	for (auto* dxRenderObj : { &this->dxRenderObjImage }) {
		//		dxRenderObj->psConstantBufferData.currentNits = { this->currentNits, 0.0f, 0.0f, 0.0f };
		//		d3dCtx->UpdateSubresource(dxRenderObj->psConstantBuffer.Get(), 0, nullptr, &dxRenderObj->psConstantBufferData, 0, 0);
		//	}
		//}
	}

	// Helper method to clear the back buffers.
	void SimpleSceneRenderer::Clear() {
		//HRESULT hr = S_OK;

		//// Clear the views.
		//auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		//auto dxCtx = dxDev->LockContext();
		//auto d3dCtx = dxCtx->D3D();

		//d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Black);

		//ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
		//d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

		//// Set the viewport.
		//auto viewport = this->swapChainPanel->GetScreenViewport();
		//d3dCtx->RSSetViewports(1, &viewport);
	}

	void SimpleSceneRenderer::Render() {
		// Don't try to render anything before the first Update.
		if (this->renderLoopTimer.GetFrameCount() == 0) {
			return;
		}

		//this->Clear();

		this->RenderHDRScene();
	}

	void SimpleSceneRenderer::RenderHDRScene() {
		HRESULT hr = S_OK;
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDevice = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		//
		// Render the HDR scene with values larger than 1.0f, which will be perceived as bright
		//
		
		//{
		//	// Render ObjImage texture to ObjProxy RTV (8:8:8:8).
		//	auto renderTargetView = this->dxRenderObjProxy->GetObj()->textureRTV;
		//	d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::DarkGreen);

		//	ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
		//	d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

		//	auto viewport = this->swapChainPanel->GetScreenViewport();
		//	d3dCtx->RSSetViewports(1, &viewport);

		//	auto& dxRenderObj = this->dxRenderObjImage->GetObj();
		//	this->quadRenderer.Draw(dxRenderObj->textureSRV, dxRenderObj.get(), [&] {
		//		d3dCtx->VSSetShader(dxRenderObj->vertexShader.Get(), nullptr, 0);
		//		d3dCtx->PSSetShader(dxRenderObj->pixelShader.Get(), nullptr, 0);
		//		d3dCtx->VSSetConstantBuffers(0, 1, dxRenderObj->vsConstantBuffer.GetAddressOf());
		//		d3dCtx->PSSetConstantBuffers(0, 1, dxRenderObj->psConstantBuffer.GetAddressOf());
		//		});
		//}


		{
			// Render ObjProxy texture (8:8:8:8) to ObjHdrQuad RTV (16:16:16:16).
			// Render ObjImage texture (8:8:8:8) to ObjHdrQuad RTV (16:16:16:16).
			auto renderTargetView = this->dxRenderObjHdrQuad->GetObj()->textureRTV;
			d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Black);

			ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
			d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

			auto viewport = this->swapChainPanel->GetScreenViewport();
			d3dCtx->RSSetViewports(1, &viewport);

			//auto& dxRenderObj = this->dxRenderObjProxy->GetObj();
			auto& dxRenderObj = this->dxRenderObjImage->GetObj();
			this->quadRenderer.Draw(dxRenderObj->textureSRV, dxRenderObj.get(), [&] {
				d3dCtx->VSSetShader(dxRenderObj->vertexShader.Get(), nullptr, 0);
				d3dCtx->PSSetShader(dxRenderObj->pixelShader.Get(), nullptr, 0);
				d3dCtx->VSSetConstantBuffers(0, 1, dxRenderObj->vsConstantBuffer.GetAddressOf());
				d3dCtx->PSSetConstantBuffers(0, 1, dxRenderObj->psConstantBuffer.GetAddressOf());
				});
		}

		//
		// Process the HDR scene so that the swapchains can correctly be sent to HDR or SDR display
		//
		{
			// Render ObjHdrQuad texture (16:16:16:16) to swapChain RTV (10:10:10:2).
			// Render ObjHdrQuad texture (16:16:16:16) to swapChain Proxy RTV (8:8:8:8).
			auto renderTargetView = this->swapChainPanel->GetRenderTargetView();
			d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);

			ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
			d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

			auto viewport = this->swapChainPanel->GetScreenViewport();
			d3dCtx->RSSetViewports(1, &viewport);

			// Render a fullscreen quad and apply the HDR/SDR shaders.
			auto& dxRenderObj = this->dxRenderObjHdrQuad->GetObj();
			this->quadRenderer.Draw(dxRenderObj->textureSRV, dxRenderObj.get(), [&] {
				d3dCtx->VSSetShader(dxRenderObj->vertexShader.Get(), nullptr, 0);
				d3dCtx->VSSetConstantBuffers(0, 1, dxRenderObj->vsConstantBuffer.GetAddressOf());

				if (this->swapChainPanel->GetColorSpace() == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {
					d3dCtx->PSSetShader(dxRenderObj->pixelShaderHDR.Get(), nullptr, 0);
					d3dCtx->PSSetConstantBuffers(0, 1, dxRenderObj->psConstantBuffer.GetAddressOf());
				}
				else {
					d3dCtx->PSSetShader(dxRenderObj->pixelShaderToneMap.Get(), nullptr, 0);
				}
				});
		}

		//this->swapChainPanel->DrawProxyTexture();

		ID3D11ShaderResourceView* nullrtv[] = { nullptr };
		d3dCtx->PSSetShaderResources(0, _countof(nullrtv), nullrtv);
	}
}