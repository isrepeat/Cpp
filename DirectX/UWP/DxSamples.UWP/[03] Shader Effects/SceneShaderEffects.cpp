#include "SceneShaderEffects.h"
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/Helpers.h>
#include <Helpers/FileSystem.h>

#include "Tools.h"
#include <DirectXMath.h>
#include <fstream>

#pragma comment(lib, "dxguid.lib")

const std::filesystem::path g_ProjectRootNamespace = L"" PP_STRINGIFY(MSBuildProject__RootNamespace);


namespace DxSamples {
	SceneShaderEffects::SceneShaderEffects(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel)
		: swapChainPanel{ swapChainPanel }
		, dxLinkingGraph{ std::make_shared<H::Dx::DxLinkingGraph>(this->swapChainPanel->GetDxDevice()) }
		, renderPipeline{ this->swapChainPanel }
	{
		HRESULT hr = S_OK;

		this->CreateDeviceDependentResources();
		this->CreateWindowSizeDependentResources();
	}

	SceneShaderEffects::~SceneShaderEffects()
	{}

	// Executes basic render loop.
	void SceneShaderEffects::Render() {
		this->renderLoopTimer.Tick([this] {
			this->Update();
			});

		this->RenderInternal();
	}

	void SceneShaderEffects::OnWindowSizeChanged(H::Size newSize) {
		this->swapChainPanel->SetLogicalSize(static_cast<H::Size_f>(newSize));
		this->CreateWindowSizeDependentResources();
	}


	void SceneShaderEffects::CreateDeviceDependentResources() {
		HRESULT hr = S_OK;

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();

		DxRenderObjImage::Params dxRenderObjImageParams;
		dxRenderObjImageParams.imagePath = L"Textures\\Dune_screenshot_24s.png";
		dxRenderObjImageParams.vertexShader = g_ProjectRootNamespace / L"Image_VS.cso";
		dxRenderObjImageParams.pixelShader = g_ProjectRootNamespace / L"Image_PS.cso";
		this->dxRenderImageObj = std::make_unique<DxRenderObjImage>(this->swapChainPanel, dxRenderObjImageParams);
		
		this->dxRenderProxyObjects.push_back(std::make_unique<H::Dx::details::DxRenderObjProxy>(this->swapChainPanel));
		this->dxRenderProxyObjects.push_back(std::make_unique<H::Dx::details::DxRenderObjProxy>(this->swapChainPanel));
		this->dxRenderProxyObjects.push_back(std::make_unique<H::Dx::details::DxRenderObjProxy>(this->swapChainPanel));
		this->dxRenderProxyObjects.push_back(std::make_unique<H::Dx::details::DxRenderObjProxy>(this->swapChainPanel));
		this->dxRenderProxyObjects.push_back(std::make_unique<H::Dx::details::DxRenderObjProxy>(this->swapChainPanel));
		this->dxRenderProxyObjects.push_back(std::make_unique<H::Dx::details::DxRenderObjProxy>(this->swapChainPanel));
		this->dxRenderProxyObjects.push_back(std::make_unique<H::Dx::details::DxRenderObjProxy>(this->swapChainPanel));
		this->dxRenderProxyObjects.push_back(std::make_unique<H::Dx::details::DxRenderObjProxy>(this->swapChainPanel));


		using H::Dx::DxVertexShaderGraphDesc;
		using H::Dx::DxPixelShaderGraphDesc;
		using H::Dx::DxConstantBufferBase;
		using H::Dx::HlslModule;
		using H::Dx::HlslFunction;
		using H::Dx::Slot;

		//
		// Vertex Shader Graph Description
		//
		DxVertexShaderGraphDesc vertexShaderGraphDesc;
		vertexShaderGraphDesc.vertexInputLayout = {
			{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		vertexShaderGraphDesc.shaderInputParameters = {
			{ "inputPosition", "SV_POSITION", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1, 4, D3D_INTERPOLATION_LINEAR, D3D_PF_IN, 0, 0, 0, 0 },
			{ "inputUV", "TEXCOORD0", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1, 2, D3D_INTERPOLATION_LINEAR, D3D_PF_IN, 0, 0, 0, 0}
		};
		vertexShaderGraphDesc.shaderOutputParameters = {
			{ "outputPosition", "SV_POSITION", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1, 4, D3D_INTERPOLATION_UNDEFINED, D3D_PF_OUT, 0, 0, 0, 0},
			{ "outputUV","TEXCOORD0",D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1, 2, D3D_INTERPOLATION_UNDEFINED, D3D_PF_OUT, 0, 0, 0, 0 },
		};
		{
			HlslModule& hlslModule = this->hlslVertexModule1;
			hlslModule.hlslFile = g_ProjectRootNamespace / "VertexModule1.VS.hlsl";
			hlslModule.hlslFunctions = {
				HlslFunction{
					"VertexFunctionA", {
						{Slot::_0, Slot::_0},
						{Slot::_1, Slot::_1},
					}
				},
				HlslFunction{
					"VertexFunctionB", {
						{Slot::_0, Slot::_0},
						{Slot::_1, Slot::_1},
					}
				},
			};
			hlslModule.AddConstantBuffer<VS_CONSTANT_BUFFER_MUDULE_A>("first", 0, 0, 0, this->swapChainPanel->GetDxDevice());
			hlslModule.AddConstantBuffer<VS_CONSTANT_BUFFER_MUDULE_A>("second", 1, 1, 0, this->swapChainPanel->GetDxDevice());
			vertexShaderGraphDesc.hlslModules.push_back(hlslModule);
		}
		{
			HlslModule& hlslModule = this->hlslVertexModule2;
			hlslModule.hlslFile = g_ProjectRootNamespace / "VertexModule2.VS.hlsl";
			hlslModule.hlslFunctions = {
				{
					"VertexFunctionA", {
						{Slot::_0, Slot::_0},
						{Slot::_1, Slot::_1},
					}
				},
			};
			vertexShaderGraphDesc.hlslModules.push_back(hlslModule);
		}
		this->dxLinkingGraph->CreateVertexShaderFromGraphDesc(vertexShaderGraphDesc);
		

		//
		// Pixel Shader Graph Description
		//
		DxPixelShaderGraphDesc pixelShaderGraphDesc;
		pixelShaderGraphDesc.shaderInputParameters = {
			{ "inputPosition", "SV_POSITION", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1, 4, D3D_INTERPOLATION_UNDEFINED, D3D_PF_IN, 0, 0, 0, 0},
			{ "inputUV","TEXCOORD0",D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1, 2, D3D_INTERPOLATION_UNDEFINED, D3D_PF_IN, 0, 0, 0, 0 },
		};
		pixelShaderGraphDesc.shaderOutputParameters = {
			{"outputColor", "SV_TARGET", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1, 4, D3D_INTERPOLATION_UNDEFINED, D3D_PF_OUT, 0, 0, 0, 0}
		};
		{
			HlslModule& hlslModule = this->hlslPixelModule1;
			hlslModule.hlslFile = g_ProjectRootNamespace / "PixelModule1.PS.hlsl";
			hlslModule.hlslFunctions = {
				{
					"PixelFunctionA", {
						{Slot::_0, Slot::_0},
						{Slot::_1, Slot::_1},
					}
				},
			};
			hlslModule.bindedResource = HlslModule::BindResource{ 0, 0, 1 };
			hlslModule.bindedSampler = HlslModule::BindSampler{ 0, 0, 1 };
			pixelShaderGraphDesc.hlslModules.push_back(hlslModule);
		}
		{
			HlslModule& hlslModule = this->hlslPixelModule2;
			hlslModule.hlslFile = g_ProjectRootNamespace / "PixelModule1.PS.hlsl";
			hlslModule.hlslFunctions = {
				{
					"PixelFunctionA", {
						{Slot::_Return, Slot::_0}
					}
				},
			};
			pixelShaderGraphDesc.hlslModules.push_back(hlslModule);
		}
		this->dxLinkingGraph->CreatePixelShaderFromGraphDesc(pixelShaderGraphDesc);
	}

	void SceneShaderEffects::CreateWindowSizeDependentResources() {
		this->dxRenderImageObj->CreateWindowSizeDependentResources();
		for (auto& dxRenderProxyObj : this->dxRenderProxyObjects) {
			dxRenderProxyObj->CreateWindowSizeDependentResources();
		}
	}

	void SceneShaderEffects::ReleaseDeviceDependentResources() {
		this->dxRenderImageObj->ReleaseDeviceDependentResources();
		for (auto& dxRenderProxyObj : this->dxRenderProxyObjects) {
			dxRenderProxyObj->ReleaseDeviceDependentResources();
		}
	}

	void SceneShaderEffects::Update() {
		//DirectX::XMStoreFloat4x4(
		//	&this->hlslVertexModule1.GetConstantBuffer<VS_CONSTANT_BUFFER_MUDULE_A>("first")->constantBufferData.mWorldViewProj,
		//	DirectX::XMMatrixTranspose(
		//		DirectX::XMMatrixIdentity()
		//	)
		//);	

		//DirectX::XMStoreFloat4x4(
		//	&this->hlslVertexModule1.GetConstantBuffer<VS_CONSTANT_BUFFER_MUDULE_A>("second")->constantBufferData.mWorldViewProj,
		//	DirectX::XMMatrixTranspose(
		//		DirectX::XMMatrixTranslation(0.0, 0, 0)
		//	)
		//);
	}

	void SceneShaderEffects::Clear() {
		//HRESULT hr = S_OK;

		//// Clear the views.
		//auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		//auto dxCtx = dxDev->LockContext();
		//auto d3dCtx = dxCtx->D3D();

		//auto renderTargetView = this->swapChainPanel->GetRenderTargetView();
		//d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);
		//
		//ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
		//d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

		//auto viewport = this->swapChainPanel->GetScreenViewport();
		//d3dCtx->RSSetViewports(1, &viewport);
	}

	void SceneShaderEffects::RenderInternal() {
		// Don't try to render anything before the first Update.
		if (this->renderLoopTimer.GetFrameCount() == 0) {
			return;
		}

		this->Clear();
		this->RenderScene();
	}

	void SceneShaderEffects::RenderScene() {
		HRESULT hr = S_OK;
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDevice = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();



		//this->renderPipeline.SetTexture(this->dxRenderObjImage->GetObj()->textureSRV);
		//this->renderPipeline.SetLinkingGraph(this->dxLinkingGraph);
		//this->renderPipeline.Draw();

		//
		// Render dxRenderImageObj texture to dxRenderProxyObjects[0].
		//
		{
			auto& dxRenderObj = this->dxRenderImageObj;
			auto& targetObject = this->dxRenderProxyObjects[0];

			auto renderTargetView = targetObject->GetObjData()->textureRTV;
			d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);

			ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
			d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

			D3D11_TEXTURE2D_DESC texDesc;
			targetObject->GetObjData()->texture->GetDesc(&texDesc);
			auto viewport = CD3D11_VIEWPORT(
				0.0f,
				0.0f,
				texDesc.Width,
				texDesc.Height
			);
			d3dCtx->RSSetViewports(1, &viewport);

			this->renderPipeline.SetTexture(dxRenderObj->GetObjData()->textureSRV);
			this->renderPipeline.SetVertexShader(
				dxRenderObj->GetObjData()->vertexShader,
				dxRenderObj->GetObjData()->vsConstantBuffer
			);
			this->renderPipeline.SetPixelShader(
				dxRenderObj->GetObjData()->pixelShader,
				dxRenderObj->GetObjData()->psConstantBuffer
			);
			dxRenderObj->UpdateBuffers();
			this->renderPipeline.Draw();

			ID3D11ShaderResourceView* nullrtv[] = { nullptr };
			d3dCtx->PSSetShaderResources(0, _countof(nullrtv), nullrtv);
		}

		//
		// Render dxRenderProxyObjPrev to dxRenderProxyObjCurrent.
		//
		for (int i = 0; i < this->dxRenderProxyObjects.size() - 1; i++) {
			auto& dxRenderProxyObjCurrent = this->dxRenderProxyObjects[i];
			auto& dxRenderProxyObjNext = this->dxRenderProxyObjects[i+1];

			auto renderTargetView = dxRenderProxyObjNext->GetObjData()->textureRTV;
			d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);

			ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
			d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

			D3D11_TEXTURE2D_DESC texDesc;
			dxRenderProxyObjNext->GetObjData()->texture->GetDesc(&texDesc);
			auto viewport = CD3D11_VIEWPORT(
				0.0f,
				0.0f,
				texDesc.Width,
				texDesc.Height
			);
			d3dCtx->RSSetViewports(1, &viewport);

			this->renderPipeline.SetTexture(dxRenderProxyObjCurrent->GetObjData()->textureSRV);
			this->renderPipeline.SetVertexShader(
				dxRenderProxyObjCurrent->GetObjData()->vertexShader,
				dxRenderProxyObjCurrent->GetObjData()->vsConstantBuffer
			);
			this->renderPipeline.SetPixelShader(
				dxRenderProxyObjCurrent->GetObjData()->pixelShader,
				dxRenderProxyObjCurrent->GetObjData()->psConstantBuffer
			);
			dxRenderProxyObjCurrent->UpdateBuffers();
			this->renderPipeline.Draw();

			ID3D11ShaderResourceView* nullrtv[] = { nullptr };
			d3dCtx->PSSetShaderResources(0, _countof(nullrtv), nullrtv);
		}

		//
		// Render last dxRenderProxyObject to swapChain RTV.
		//
		{
			auto renderTargetView = this->swapChainPanel->GetRenderTargetView();
			d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);

			ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
			d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

			auto viewport = this->swapChainPanel->GetScreenViewport();
			d3dCtx->RSSetViewports(1, &viewport);

			auto& dxRenderProxyObjLast = this->dxRenderProxyObjects.back();
			this->renderPipeline.SetTexture(dxRenderProxyObjLast->GetObjData()->textureSRV);
			this->renderPipeline.SetVertexShader(
				dxRenderProxyObjLast->GetObjData()->vertexShader,
				dxRenderProxyObjLast->GetObjData()->vsConstantBuffer
			);
			this->renderPipeline.SetPixelShader(
				dxRenderProxyObjLast->GetObjData()->pixelShader,
				dxRenderProxyObjLast->GetObjData()->psConstantBuffer
			);
			dxRenderProxyObjLast->UpdateBuffers();
			this->renderPipeline.Draw();

			ID3D11ShaderResourceView* nullrtv[] = { nullptr };
			d3dCtx->PSSetShaderResources(0, _countof(nullrtv), nullrtv);
		}

		// ...
		// swapChain proxy texture (16:16:16:16) to swapChain RTV (10:10:10:2).
	}
}