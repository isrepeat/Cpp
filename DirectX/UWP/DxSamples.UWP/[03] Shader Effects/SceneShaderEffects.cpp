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
		, dxLinkageGraphPipeline{ this->swapChainPanel->GetDxDevice() }
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


		//hr = D3DCreateFunctionLinkingGraph(0, this->vertexShaderGraph.GetAddressOf());
		//H::System::ThrowIfFailed(hr);

		////Create a linker
		//hr = D3DCreateLinker(this->linker.GetAddressOf());
		//H::System::ThrowIfFailed(hr);

		std::string d3dLastError;
		{
			auto vertexShaderGraph = this->dxLinkageGraphPipeline.GetVertexShaderGraph();

			auto vsModule1 = this->dxLinkageGraphPipeline.AddModule("Function1.VS.hlsl");
			vsModule1.shaderLibraryInstance->BindResource(0, 0, 1);
			vsModule1.shaderLibraryInstance->BindSampler(0, 0, 1);

			auto vsModule2 = this->dxLinkageGraphPipeline.AddModule("Function2.VS.hlsl");
			vsModule2.shaderLibraryInstance->BindResource(0, 0, 1);
			vsModule2.shaderLibraryInstance->BindSampler(0, 0, 1);

			this->dxLinkageGraphPipeline.SetInputSignature();
			auto vertexShaderInputNode = this->dxLinkageGraphPipeline.GetVertexShaderInputNode();

			this->dxLinkageGraphPipeline.CallFunction(vsModule1, "VertexFunction");
			hr = vertexShaderGraph->PassValue(vertexShaderInputNode.Get(), 0, vsModule1.shaderCallFunctionNode.Get(), 0);
			H::System::ThrowIfFailed(hr);
			hr = vertexShaderGraph->PassValue(vertexShaderInputNode.Get(), 1, vsModule1.shaderCallFunctionNode.Get(), 1);
			H::System::ThrowIfFailed(hr);

			this->dxLinkageGraphPipeline.CallFunction(vsModule2, "VertexFunctionNew");
			hr = vertexShaderGraph->PassValue(vsModule1.shaderCallFunctionNode.Get(), 0, vsModule2.shaderCallFunctionNode.Get(), 0);
			H::System::ThrowIfFailed(hr);
			hr = vertexShaderGraph->PassValue(vsModule1.shaderCallFunctionNode.Get(), 1, vsModule2.shaderCallFunctionNode.Get(), 1);
			H::System::ThrowIfFailed(hr);

			this->dxLinkageGraphPipeline.SetOutputSignature();
			auto vertexShaderOutputNode = this->dxLinkageGraphPipeline.GetVertexShaderOutputNode();

			hr = vertexShaderGraph->PassValue(vsModule2.shaderCallFunctionNode.Get(), 0, vertexShaderOutputNode.Get(), 0);
			H::System::ThrowIfFailed(hr);
			hr = vertexShaderGraph->PassValue(vsModule2.shaderCallFunctionNode.Get(), 1, vertexShaderOutputNode.Get(), 1);
			H::System::ThrowIfFailed(hr);

			this->dxLinkageGraphPipeline.LogVertexShaderGraph();
			this->dxLinkageGraphPipeline.CreateVertexShader();



		}
	}

	void SceneShaderEffects::CreateWindowSizeDependentResources() {
		this->dxRenderObjImage->CreateWindowSizeDependentResources();
	}

	void SceneShaderEffects::ReleaseDeviceDependentResources() {
		this->dxRenderObjImage->ReleaseDeviceDependentResources();
	}

	void SceneShaderEffects::Update() {
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

		//
		// Render dxRenderObjImage texture to swapChain RTV.
		//
		{
			auto& dxRenderObj = this->dxRenderObjImage;
			dxRenderObj->UpdateBuffers();

			this->renderPipeline.SetTexture(dxRenderObj->GetObj()->textureSRV);

			this->renderPipeline.SetInputLayout(
				//this->inputLayout
				this->dxLinkageGraphPipeline.GetInputLayout()
			);

			this->renderPipeline.SetVertexShader(
				//this->vertexShader
				this->dxLinkageGraphPipeline.GetVertexShader()
			);

			//this->renderPipeline.SetVertexShader(
			//	dxRenderObj->GetObj()->vertexShader,
			//	dxRenderObj->GetObj()->vsConstantBuffer
			//);

			this->renderPipeline.SetPixelShader(
				dxRenderObj->GetObj()->pixelShader,
				dxRenderObj->GetObj()->psConstantBuffer
			);

			this->renderPipeline.Draw();
		}


		ID3D11ShaderResourceView* nullrtv[] = { nullptr };
		d3dCtx->PSSetShaderResources(0, _countof(nullrtv), nullrtv);

		// ...
		// swapChain proxy texture (16:16:16:16) to swapChain RTV (10:10:10:2).
	}
}




//auto vsFileFunction1 = H::FS::ReadFile(L"Function1.VS.hlsl");
//
//Microsoft::WRL::ComPtr<ID3DBlob> vsCodeBlob;
//Microsoft::WRL::ComPtr<ID3DBlob> vsErrorBlob;
//hr = D3DCompile(
//	vsFileFunction1.data(),
//	vsFileFunction1.size(),
//	//"ShaderModuleVS",
//	NULL,
//	NULL,
//	NULL,
//	NULL,
//	"lib_5_0",
//	D3DCOMPILE_OPTIMIZATION_LEVEL3,
//	0,
//	&vsCodeBlob,
//	&vsErrorBlob
//);
//
//if (vsErrorBlob) {
//	d3dLastError = (char*)vsErrorBlob->GetBufferPointer();
//}
//H::System::ThrowIfFailed(hr);
//
//
//// Load the compiled library code into a module object.
//hr = D3DLoadModule(
//	vsCodeBlob->GetBufferPointer(),
//	vsCodeBlob->GetBufferSize(),
//	this->shaderLibrary.GetAddressOf()
//);
//H::System::ThrowIfFailed(hr);
//
//
//// Create an instance of the library and define resource bindings for it.
//// In this sample we use the same slots as the source library however this is not required.
//hr = this->shaderLibrary->CreateInstance("", this->shaderLibraryInstance.GetAddressOf());
//H::System::ThrowIfFailed(hr);
//// HRESULTs for Bind methods are intentionally ignored as compiler optimizations may eliminate the source
//// bindings. In these cases, the Bind operation will fail, but the final shader will function normally.
//this->shaderLibraryInstance->BindResource(0, 0, 1);
//this->shaderLibraryInstance->BindSampler(0, 0, 1);
//
//
////Define the vertex shader input layout
//static const D3D11_PARAMETER_DESC vertexShaderInputParameters[] =
//{
//	{ "inputPosition", "SV_POSITION", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1,4,D3D_INTERPOLATION_LINEAR, D3D_PF_IN,0,0,0,0 },
//	{ "inputUV", "TEXCOORD0", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1,2,D3D_INTERPOLATION_LINEAR,D3D_PF_IN,0,0,0,0}
//};
//
//hr = this->vertexShaderGraph->SetInputSignature(
//	vertexShaderInputParameters,
//	ARRAYSIZE(vertexShaderInputParameters),
//	this->vertexShaderInputNode.GetAddressOf()
//);
//H::System::ThrowIfFailed(hr);
//
//
//// Create a node for the main VertexFunction call using the output of the helper functions.
//hr = this->vertexShaderGraph->CallFunction(
//	"",
//	this->shaderLibrary.Get(),
//	"VertexFunction",
//	this->vertexFunctionCallNode.GetAddressOf()
//);
//H::System::ThrowIfFailed(hr);
//
//// Define the graph edges from the input node and helper function nodes.
//hr = this->vertexShaderGraph->PassValue(this->vertexShaderInputNode.Get(), 0, this->vertexFunctionCallNode.Get(), 0);
//H::System::ThrowIfFailed(hr);
//hr = this->vertexShaderGraph->PassValue(this->vertexShaderInputNode.Get(), 1, this->vertexFunctionCallNode.Get(), 1);
//H::System::ThrowIfFailed(hr);
//
////Define the output layout for the vertex function
//static const D3D11_PARAMETER_DESC vertexShaderOutputParameters[] =
//{
//	{ "outputPosition", "SV_POSITION", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1,4,D3D_INTERPOLATION_UNDEFINED, D3D_PF_OUT, 0,0,0,0},
//	{ "outputUV","TEXCOORD0",D3D_SVT_FLOAT, D3D_SVC_VECTOR,1,2,D3D_INTERPOLATION_UNDEFINED,D3D_PF_OUT,0,0,0,0 },
//};
//
////Create the vertex output node
//hr = this->vertexShaderGraph->SetOutputSignature(
//	vertexShaderOutputParameters,
//	ARRAYSIZE(vertexShaderOutputParameters),
//	this->vertexShaderOutputNode.GetAddressOf()
//);
//H::System::ThrowIfFailed(hr);
//
////Pass the value from the function node to the output node
//hr = this->vertexShaderGraph->PassValue(this->vertexFunctionCallNode.Get(), 0, this->vertexShaderOutputNode.Get(), 0);
//H::System::ThrowIfFailed(hr);
//hr = this->vertexShaderGraph->PassValue(this->vertexFunctionCallNode.Get(), 1, this->vertexShaderOutputNode.Get(), 1);
//H::System::ThrowIfFailed(hr);
//
//
////Finalize the the vertex shader graph
//hr = this->vertexShaderGraph->CreateModuleInstance(
//	this->vertexShaderGraphInstance.GetAddressOf(),
//	nullptr
//);
//H::System::ThrowIfFailed(hr);
//
//////Create a linker
////hr = D3DCreateLinker(this->linker.GetAddressOf());
////H::System::ThrowIfFailed(hr);
//
////Hook up the shader library instance
//hr = this->linker->UseLibrary(this->shaderLibraryInstance.Get());
//H::System::ThrowIfFailed(hr);
//
////Link the vertex shader. This looks mostly normal, frankly. Interesting. TODO: Something goes wrong here. What? #HelpfulError
//ID3DBlob* vertexShaderBlob = nullptr;
//ID3DBlob* errorBlob = nullptr;
////Error occurs here
//hr = this->linker->Link(
//	this->vertexShaderGraphInstance.Get(),
//	"main",
//	"vs_5_0",
//	0,
//	&vertexShaderBlob,
//	&errorBlob
//);
//H::System::ThrowIfFailed(hr);
//
//
//
//static const D3D11_INPUT_ELEMENT_DESC inputElementDesc[2] = {
//	{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
//	{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//};
//
//hr = d3dDev->CreateInputLayout(
//	inputElementDesc,
//	ARRAYSIZE(inputElementDesc),
//	vertexShaderBlob->GetBufferPointer(),
//	vertexShaderBlob->GetBufferSize(),
//	this->inputLayout.GetAddressOf()
//);
//H::System::ThrowIfFailed(hr);
//
//
////Create the vertex shader. Business as usual?
//hr = d3dDev->CreateVertexShader(
//	vertexShaderBlob->GetBufferPointer(),
//	vertexShaderBlob->GetBufferSize(),
//	nullptr,
//	this->vertexShader.GetAddressOf()
//);
//H::System::ThrowIfFailed(hr);
