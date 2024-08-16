#include "HDRScene.h"
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/Helpers.h>

#include "Tools.h"
#include <DirectXMath.h>
#include <fstream>

#pragma comment(lib, "dxguid.lib")

namespace DxSamples {
	HDRScene::HDRScene(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel)
		: swapChainPanel{ swapChainPanel }
		//, quadRenderer{ swapChainPanel->GetDxDevice() }
		, fullScreenQuad{ swapChainPanel->GetDxDevice() }
		, currentNits{ 1500.0f }
	{
		HRESULT hr = S_OK;

		{
			HRESULT hr = S_OK;

			D3D11_RASTERIZER_DESC rsDesc = {};
			rsDesc.FillMode = D3D11_FILL_SOLID;
			rsDesc.CullMode = D3D11_CULL_BACK;
			rsDesc.FrontCounterClockwise = FALSE;
			rsDesc.DepthBias = 0;
			rsDesc.SlopeScaledDepthBias = 0.0f;
			rsDesc.DepthBiasClamp = 0.0f;
			rsDesc.DepthClipEnable = TRUE;
			rsDesc.ScissorEnable = TRUE;
			//rsDesc.ScissorEnable = FALSE;
			rsDesc.MultisampleEnable = FALSE;
			rsDesc.AntialiasedLineEnable = FALSE;

			auto dxDev = swapChainPanel->GetDxDevice()->Lock();
			auto d3dDev = dxDev->GetD3DDevice();
			hr = d3dDev->CreateRasterizerState(&rsDesc, this->rsState.GetAddressOf());
			H::System::ThrowIfFailed(hr);
		}

		this->CreateDeviceDependentResources();
		this->CreateWindowSizeDependentResources();
	}

	HDRScene::~HDRScene()
	{}

	// Executes basic render loop.
	void HDRScene::Render() {
		this->renderLoopTimer.Tick([this] {
			this->Update();
		});

		this->RenderInternal();
	}

	void HDRScene::OnWindowSizeChanged(H::Size newSize) {
		LOG_DEBUG_D("OnWindowSizeChanged(newSize = [{}, {}])", newSize.width, newSize.height);
		this->swapChainPanel->SetLogicalSize(static_cast<H::Size_f>(newSize));
		this->CreateWindowSizeDependentResources();
	}


	void HDRScene::CreateDeviceDependentResources() {
		HRESULT hr = S_OK;

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();
		
		this->dxRenderObjImage = std::make_unique<DxRenderObjImage>(this->swapChainPanel);
		//this->dxRenderObjHdrQuad = std::make_unique<DxRenderObjHdrQuad>(this->swapChainPanel);

		H::Dx::details::DxRenderObjHDR::Params paramsProxyLocal;
		paramsProxyLocal.pixelShaderToneMap = g_shaderLoadDir / L"Proxy1_PS.cso";
		dxRenderObjProxyLocal = std::make_unique<H::Dx::details::DxRenderObjHDR>(swapChainPanel->GetDxDevice(), paramsProxyLocal);
		dxRenderObjProxyLocal->CreateTextureHDR(H::Size{ 50, 50 });

		H::Dx::details::DxRenderObjHDR::Params paramsProxy1;
		paramsProxy1.pixelShaderToneMap = g_shaderLoadDir / L"Proxy1_PS.cso";
		this->dxRenderObjProxy1 = std::make_unique<H::Dx::details::DxRenderObjHDR>(swapChainPanel->GetDxDevice(), paramsProxy1);

		H::Dx::details::DxRenderObjHDR::Params paramsProxy2;
		paramsProxy2.pixelShaderToneMap = g_shaderLoadDir / L"Proxy2_PS.cso";
		this->dxRenderObjProxy2 = std::make_unique<H::Dx::details::DxRenderObjHDR>(swapChainPanel->GetDxDevice(), paramsProxy2);

		//this->swapChainPanel->SetLogicalSize(H::Size_f{400, 220});
		//this->dxRenderObjProxy1->CreateWindowSizeDependentResources();
		this->dxRenderObjProxy1->CreateTextureHDR(H::Size{ 11,11 });


	}

	void HDRScene::CreateWindowSizeDependentResources() {
		//this->dxRenderObjProxy1->CreateWindowSizeDependentResources();
		this->dxRenderObjImage->CreateWindowSizeDependentResources();
	}

	void HDRScene::ReleaseDeviceDependentResources() {
		this->dxRenderObjProxy1->ReleaseDeviceDependentResources();
		this->dxRenderObjProxy2->ReleaseDeviceDependentResources();
		this->dxRenderObjImage->ReleaseDeviceDependentResources();
		//this->dxRenderObjHdrQuad->ReleaseDeviceDependentResources();
	}

	void HDRScene::Update() {
		//using DirectX::Keyboard;

		//auto kb = this->keyboard->GetState();
		//this->keyboardButtons.Update(kb);

		//bool nitsChanged = false;

		//if (kb.Escape) {
		//	//ExitSample();
		//}

		//if (this->keyboardButtons.IsKeyPressed(Keyboard::Space)) {
		//}

		//if (this->keyboardButtons.IsKeyPressed(Keyboard::OemMinus) || this->keyboardButtons.IsKeyPressed(Keyboard::Subtract)) {
		//	nitsChanged = true;
		//	this->currentNits -= 200.0f;
		//	this->currentNits = std::max(this->currentNits, 200.0f);
		//}

		//if (this->keyboardButtons.IsKeyPressed(Keyboard::OemPlus) || this->keyboardButtons.IsKeyPressed(Keyboard::Add)) {
		//	nitsChanged = true;
		//	this->currentNits += 200.0f;
		//	this->currentNits = std::max(this->currentNits, DX::c_MaxNitsFor2084);
		//}

		//if (kb.Down || kb.Left) {
		//}

		//if (kb.Up || kb.Right){
		//}

		////// Update constant buffer
		////if (nitsChanged) {
		////	auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		////	auto dxCtx = dxDev->LockContext();
		////	auto d3dCtx = dxCtx->D3D();

		////	for (auto* dxRenderObj : { &this->dxRenderObjImage }) {
		////		dxRenderObj->psConstantBufferData.currentNits = { this->currentNits, 0.0f, 0.0f, 0.0f };
		////		d3dCtx->UpdateSubresource(dxRenderObj->psConstantBuffer.Get(), 0, nullptr, &dxRenderObj->psConstantBufferData, 0, 0);
		////	}
		////}
	}

	// Helper method to clear the back buffers.
	void HDRScene::Clear() {
		//HRESULT hr = S_OK;

		//// Clear the views.
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		//d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Black);

		//ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
		//d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

		//// Set the viewport.
		auto viewport = this->swapChainPanel->GetScreenViewport();
		d3dCtx->RSSetViewports(1, &viewport);
	}

	void HDRScene::RenderInternal() {
		// Don't try to render anything before the first Update.
		if (this->renderLoopTimer.GetFrameCount() == 0) {
			return;
		}

		//this->Clear();
		this->RenderHDRScene();
	}

	void HDRScene::RenderHDRScene() {
		HRESULT hr = S_OK;
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();
		auto d3dCtx = dxCtx->D3D();

		////
		//// Render the HDR scene with values larger than 1.0f, which will be perceived as bright
		////

		//{
		//	// Render ObjImage texture (x:x:x:x) to ObjHdrQuad RTV (16:16:16:16).
		//	auto renderTargetView = this->dxRenderObjHdrQuad->GetObj()->textureRTV;
		//	//auto renderTargetView = this->swapChainPanel->GetRenderTargetView();
		//	d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Purple);

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

		////
		//// Process the HDR scene so that the swapchains can correctly be sent to HDR or SDR display
		////
		//{
		//	// Render ObjHdrQuad texture (16:16:16:16) to swapChain proxy RTV (16:16:16:16).
		//	auto renderTargetView = this->swapChainPanel->GetRenderTargetView();
		//	d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);

		//	ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
		//	d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

		//	auto viewport = this->swapChainPanel->GetScreenViewport();
		//	d3dCtx->RSSetViewports(1, &viewport);

		//	// Render a fullscreen quad and apply the HDR/SDR shaders.
		//	auto& dxRenderObj = this->dxRenderObjHdrQuad->GetObj();
		//	this->quadRenderer.Draw(dxRenderObj->textureSRV, dxRenderObj.get(), [&] {
		//		d3dCtx->VSSetShader(dxRenderObj->vertexShader.Get(), nullptr, 0);
		//		d3dCtx->VSSetConstantBuffers(0, 1, dxRenderObj->vsConstantBuffer.GetAddressOf());

		//		if (this->swapChainPanel->GetColorSpace() == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {
		//			d3dCtx->PSSetShader(dxRenderObj->pixelShaderHDR.Get(), nullptr, 0);
		//			d3dCtx->PSSetConstantBuffers(0, 1, dxRenderObj->psConstantBuffer.GetAddressOf());
		//		}
		//		else {
		//			d3dCtx->PSSetShader(dxRenderObj->pixelShaderToneMap.Get(), nullptr, 0);
		//		}
		//		});
		//}


		//ID3D11ShaderResourceView* nullrtv[] = { nullptr };
		//d3dCtx->PSSetShaderResources(0, _countof(nullrtv), nullrtv);

		// ...
		// swapChain->Present render internal own proxy texture (16:16:16:16) to swapChain RTV (10:10:10:2).

		d2dCtx->BeginDraw();
		
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rsStatePrev;
		d3dCtx->RSGetState(rsStatePrev.GetAddressOf());
		
		d3dCtx->RSSetState(this->rsState.Get());

		hr = d2dCtx->Flush();
		H::System::ThrowIfFailed(hr);


		this->dxRenderObjProxy1->CreateTextureHDR({ 200, 200 });

		{
			auto renderTargetView = this->dxRenderObjProxy1->GetObj()->textureRTV;
			d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Purple);

			ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
			d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

			auto swapChainViewport = this->swapChainPanel->GetScreenViewport();

			D3D11_TEXTURE2D_DESC texDesc;
			this->dxRenderObjProxy1->GetObj()->texture->GetDesc(&texDesc);
			D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(
				0.0f,
				0.0f,
				texDesc.Width,
				texDesc.Height
			);
			d3dCtx->RSSetViewports(1, &viewport);

			auto baseTextureSRV = this->dxRenderObjImage->GetObj()->textureSRV;

			auto& dxRenderObj = this->dxRenderObjProxy1->GetObj();
			this->fullScreenQuad.Draw(baseTextureSRV, dxRenderObj.get(), [&] {
				d3dCtx->VSSetShader(dxRenderObj->vertexShader.Get(), nullptr, 0);
				d3dCtx->PSSetShader(dxRenderObj->pixelShaderToneMap.Get(), nullptr, 0);
				d3dCtx->VSSetConstantBuffers(0, 1, dxRenderObj->vsConstantBuffer.GetAddressOf());
				d3dCtx->PSSetConstantBuffers(0, 1, dxRenderObj->psConstantBuffer.GetAddressOf());
				});
		}
		{
			auto renderTargetView = this->swapChainPanel->GetRenderTargetView();
			d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);

			ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
			d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

			auto viewport = this->swapChainPanel->GetScreenViewport();
			d3dCtx->RSSetViewports(1, &viewport);

			auto proxyTextureSRV = this->dxRenderObjProxy1->GetObj()->textureSRV;

			auto& dxRenderObj = this->dxRenderObjProxy2->GetObj();
			this->fullScreenQuad.Draw(proxyTextureSRV, dxRenderObj.get(), [&] {
				d3dCtx->VSSetShader(dxRenderObj->vertexShader.Get(), nullptr, 0);
				d3dCtx->PSSetShader(dxRenderObj->pixelShaderToneMap.Get(), nullptr, 0);
				d3dCtx->VSSetConstantBuffers(0, 1, dxRenderObj->vsConstantBuffer.GetAddressOf());
				d3dCtx->PSSetConstantBuffers(0, 1, dxRenderObj->psConstantBuffer.GetAddressOf());
				});
		}

		d3dCtx->RSSetState(rsStatePrev.Get());

		d2dCtx->EndDraw();
	}
}