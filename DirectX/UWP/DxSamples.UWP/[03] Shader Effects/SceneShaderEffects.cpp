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
		this->dxRenderObjImage = std::make_unique<DxRenderObjImage>(this->swapChainPanel, dxRenderObjImageParams);

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
		this->dxRenderObjImage->CreateWindowSizeDependentResources();
	}

	void SceneShaderEffects::ReleaseDeviceDependentResources() {
		this->dxRenderObjImage->ReleaseDeviceDependentResources();
	}

	void SceneShaderEffects::Update() {
		DirectX::XMStoreFloat4x4(
			&this->hlslVertexModule1.GetConstantBuffer<VS_CONSTANT_BUFFER_MUDULE_A>("first")->constantBufferData.mWorldViewProj,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixIdentity()
			)
		);	

		DirectX::XMStoreFloat4x4(
			&this->hlslVertexModule1.GetConstantBuffer<VS_CONSTANT_BUFFER_MUDULE_A>("second")->constantBufferData.mWorldViewProj,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixTranslation(0.5, 0, 0)
			)
		);
	}

	void SceneShaderEffects::Clear() {
		HRESULT hr = S_OK;

		// Clear the views.
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		auto renderTargetView = this->swapChainPanel->GetRenderTargetView();
		d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);
		
		ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
		d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

		auto viewport = this->swapChainPanel->GetScreenViewport();
		d3dCtx->RSSetViewports(1, &viewport);
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



		this->renderPipeline.SetTexture(this->dxRenderObjImage->GetObj()->textureSRV);
		this->renderPipeline.SetLinkingGraph(this->dxLinkingGraph);
		this->renderPipeline.Draw();


		////
		//// Render dxRenderObjImage texture to swapChain RTV.
		////
		//{
		//	auto& dxRenderObj = this->dxRenderObjImage;
		//	dxRenderObj->UpdateBuffers();

		//	this->renderPipeline.SetTexture(dxRenderObj->GetObj()->textureSRV);

		//	this->renderPipeline.SetInputLayout(
		//		//this->inputLayout
		//		this->dxLinkageGraphPipeline.GetInputLayout()
		//	);

		//	this->renderPipeline.SetVertexShader(
		//		//this->vertexShader
		//		this->dxLinkageGraphPipeline.GetVertexShader()
		//	);

		//	this->renderPipeline.SetPixelShader(
		//		this->dxLinkageGraphPipeline.GetPixelShader()
		//	);

		//	//this->renderPipeline.SetVertexShader(
		//	//	dxRenderObj->GetObj()->vertexShader,
		//	//	dxRenderObj->GetObj()->vsConstantBuffer
		//	//);

		//	//this->renderPipeline.SetPixelShader(
		//	//	dxRenderObj->GetObj()->pixelShader,
		//	//	dxRenderObj->GetObj()->psConstantBuffer
		//	//);

		//	this->renderPipeline.Draw();
		//}

		// TODO: write some stages for render pipiline to test performance for old logic

		ID3D11ShaderResourceView* nullrtv[] = { nullptr };
		d3dCtx->PSSetShaderResources(0, _countof(nullrtv), nullrtv);

		// ...
		// swapChain proxy texture (16:16:16:16) to swapChain RTV (10:10:10:2).
	}
}