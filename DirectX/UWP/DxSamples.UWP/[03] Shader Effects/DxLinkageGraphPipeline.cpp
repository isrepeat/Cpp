#include "DxLinkageGraphPipeline.h"
#include <Helpers/FileSystem.h>


namespace DxSamples {
	DxLinkageGraphPipeline::DxLinkageGraphPipeline(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj)
		: dxDeviceSafeObj{ dxDeviceSafeObj }
	{
	}

	void DxLinkageGraphPipeline::CreateVertexShaderFromGraphDesc(const DxVertexShaderGraphDesc& dxVertexShaderGraphDesc) {
		HRESULT hr = S_OK;

		//
		// Set input signature.
		//
		hr = this->dxVertexShaderGraph.shaderGraph->SetInputSignature(
			dxVertexShaderGraphDesc.shaderInputParameters.data(),
			dxVertexShaderGraphDesc.shaderInputParameters.size(),
			this->dxVertexShaderGraph.shaderInputNode.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		//
		// Add modules, call functions and link passing params from previous function to next.
		//
		std::vector<HlslFunction::ParamMatch> lastFunctionParamsMatch;

		for (auto& hlslModule : dxVertexShaderGraphDesc.hlslModules) {
			auto dxShaderModule = this->LoadShaderModule(hlslModule);

			hr = this->dxVertexShaderGraph.linker->UseLibrary(dxShaderModule.shaderLibraryInstance.Get());
			H::System::ThrowIfFailed(hr);

			// Call funcitons and match passed params.
			for (auto& hlslFunction : hlslModule.hlslFunctions) {
				lastFunctionParamsMatch = hlslFunction.paramsMatch; // remember last params for shaderOutputNode

				Microsoft::WRL::ComPtr<ID3D11LinkingNode> shaderCallFunctionNode;
				hr = this->dxVertexShaderGraph.shaderGraph->CallFunction(
					"",
					dxShaderModule.shaderLibrary.Get(),
					hlslFunction.functionName.c_str(),
					shaderCallFunctionNode.GetAddressOf()
				);
				H::System::ThrowIfFailed(hr);

				if (this->dxVertexShaderGraph.dxShaderModules.empty()) {
					if (dxShaderModule.shaderCallFunctionNodes.empty()) {
						LOG_ASSERT(dxVertexShaderGraphDesc.shaderInputParameters.size() == hlslFunction.paramsMatch.size()
							, "Shader input params size doesn't match params for first added hlslFunction"
						);
						this->PassValues(&this->dxVertexShaderGraph, this->dxVertexShaderGraph.shaderInputNode, shaderCallFunctionNode, hlslFunction.paramsMatch);
					}
					else {
						auto lastFunctionNode = dxShaderModule.shaderCallFunctionNodes.back();
						this->PassValues(&this->dxVertexShaderGraph, lastFunctionNode, shaderCallFunctionNode, hlslFunction.paramsMatch);
					}
				}
				else {
					auto prevModule = this->dxVertexShaderGraph.dxShaderModules.back();
					auto lastFunctionNode = prevModule.shaderCallFunctionNodes.back();
					this->PassValues(&this->dxVertexShaderGraph, lastFunctionNode, shaderCallFunctionNode, hlslFunction.paramsMatch);
				}
				dxShaderModule.shaderCallFunctionNodes.push_back(shaderCallFunctionNode);
			}
			this->dxVertexShaderGraph.dxShaderModules.push_back(std::move(dxShaderModule));
		}

		//
		// Set output signature.
		//
		// Create the vertex output node.
		hr = this->dxVertexShaderGraph.shaderGraph->SetOutputSignature(
			dxVertexShaderGraphDesc.shaderOutputParameters.data(),
			dxVertexShaderGraphDesc.shaderOutputParameters.size(),
			this->dxVertexShaderGraph.shaderOutputNode.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		auto prevModule = this->dxVertexShaderGraph.dxShaderModules.back();
		auto lastFunctionNode = prevModule.shaderCallFunctionNodes.back();
		this->PassValues(&this->dxVertexShaderGraph, lastFunctionNode, this->dxVertexShaderGraph.shaderOutputNode, lastFunctionParamsMatch);
		

		// Logs generated shader.
		{
			Microsoft::WRL::ComPtr<ID3DBlob> blobHlsl;
			hr = this->dxVertexShaderGraph.shaderGraph->GenerateHlsl(0, &blobHlsl);
			H::System::ThrowIfFailed(hr);

			LOG_DEBUG_D("vertexShaderGraph --> hlsl:\n"
				"{}"
				, std::string{ static_cast<const char*>(blobHlsl->GetBufferPointer()), blobHlsl->GetBufferSize() }
			);
		}

		//
		// Create vertex shader.
		//
		auto dxDev = this->dxDeviceSafeObj->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();

		// Finalize the the vertex shader graph.
		hr = this->dxVertexShaderGraph.shaderGraph->CreateModuleInstance(
			this->dxVertexShaderGraph.shaderGraphInstance.GetAddressOf(),
			nullptr
		);
		H::System::ThrowIfFailed(hr);

		// Link the vertex shader.
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		hr = this->dxVertexShaderGraph.linker->Link(
			this->dxVertexShaderGraph.shaderGraphInstance.Get(),
			"main",
			"vs_5_0",
			0,
			this->dxVertexShaderGraph.shaderBlob.GetAddressOf(),
			&errorBlob
		);
		if (errorBlob) {
			LOG_ERROR_D("d3dLinkerError = \"{}\"", (char*)errorBlob->GetBufferPointer());
		}
		H::System::ThrowIfFailed(hr);

		hr = d3dDev->CreateInputLayout(
			dxVertexShaderGraphDesc.vertexInputLayout.data(),
			dxVertexShaderGraphDesc.vertexInputLayout.size(),
			this->dxVertexShaderGraph.shaderBlob->GetBufferPointer(),
			this->dxVertexShaderGraph.shaderBlob->GetBufferSize(),
			this->dxVertexShaderGraph.inputLayout.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		hr = d3dDev->CreateVertexShader(
			this->dxVertexShaderGraph.shaderBlob->GetBufferPointer(),
			this->dxVertexShaderGraph.shaderBlob->GetBufferSize(),
			nullptr,
			this->dxVertexShaderGraph.vertexShader.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);
	}


	void DxLinkageGraphPipeline::CreatePixelShaderFromGraphDesc(const DxPixelShaderGraphDesc& dxPixelShaderGraphDesc) {
		HRESULT hr = S_OK;

		//
		// Set input signature.
		//
		hr = this->dxPixelShaderGraph.shaderGraph->SetInputSignature(
			dxPixelShaderGraphDesc.shaderInputParameters.data(),
			dxPixelShaderGraphDesc.shaderInputParameters.size(),
			this->dxPixelShaderGraph.shaderInputNode.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		//
		// Add modules, call functions and link passing params from previous function to next.
		//
		std::vector<HlslFunction::ParamMatch> lastFunctionParamsMatch;

		for (auto& hlslModule : dxPixelShaderGraphDesc.hlslModules) {
			auto dxShaderModule = this->LoadShaderModule(hlslModule);

			hr = this->dxPixelShaderGraph.linker->UseLibrary(dxShaderModule.shaderLibraryInstance.Get());
			H::System::ThrowIfFailed(hr);

			// Call funcitons and match passed params.
			for (auto& hlslFunction : hlslModule.hlslFunctions) {
				lastFunctionParamsMatch = hlslFunction.paramsMatch; // remember last params for shaderOutputNode

				Microsoft::WRL::ComPtr<ID3D11LinkingNode> shaderCallFunctionNode;
				hr = this->dxPixelShaderGraph.shaderGraph->CallFunction(
					"",
					dxShaderModule.shaderLibrary.Get(),
					hlslFunction.functionName.c_str(),
					shaderCallFunctionNode.GetAddressOf()
				);
				H::System::ThrowIfFailed(hr);

				if (this->dxPixelShaderGraph.dxShaderModules.empty()) {
					if (dxShaderModule.shaderCallFunctionNodes.empty()) {
						LOG_ASSERT(dxPixelShaderGraphDesc.shaderInputParameters.size() == hlslFunction.paramsMatch.size()
							, "Shader input params size doesn't match params for first added hlslFunction"
						);
						this->PassValues(&this->dxPixelShaderGraph, this->dxPixelShaderGraph.shaderInputNode, shaderCallFunctionNode, hlslFunction.paramsMatch);
					}
					else {
						auto lastFunctionNode = dxShaderModule.shaderCallFunctionNodes.back();
						this->PassValues(&this->dxPixelShaderGraph, lastFunctionNode, shaderCallFunctionNode, hlslFunction.paramsMatch);
					}
				}
				else {
					auto prevModule = this->dxPixelShaderGraph.dxShaderModules.back();
					auto lastFunctionNode = prevModule.shaderCallFunctionNodes.back();
					this->PassValues(&this->dxPixelShaderGraph, lastFunctionNode, shaderCallFunctionNode, hlslFunction.paramsMatch);
				}
				dxShaderModule.shaderCallFunctionNodes.push_back(shaderCallFunctionNode);
			}
			this->dxPixelShaderGraph.dxShaderModules.push_back(std::move(dxShaderModule));
		}

		//
		// Set output signature.
		//
		// Create the pixel output node.
		hr = this->dxPixelShaderGraph.shaderGraph->SetOutputSignature(
			dxPixelShaderGraphDesc.shaderOutputParameters.data(),
			dxPixelShaderGraphDesc.shaderOutputParameters.size(),
			this->dxPixelShaderGraph.shaderOutputNode.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		auto prevModule = this->dxPixelShaderGraph.dxShaderModules.back();
		auto lastFunctionNode = prevModule.shaderCallFunctionNodes.back();
		this->PassValues(&this->dxPixelShaderGraph, lastFunctionNode, this->dxPixelShaderGraph.shaderOutputNode, lastFunctionParamsMatch);

		// Logs generated shader.
		{
			Microsoft::WRL::ComPtr<ID3DBlob> blobHlsl;
			hr = this->dxPixelShaderGraph.shaderGraph->GenerateHlsl(0, &blobHlsl);
			H::System::ThrowIfFailed(hr);

			LOG_DEBUG_D("pixelShaderGraph --> hlsl:\n"
				"{}"
				, std::string{ static_cast<const char*>(blobHlsl->GetBufferPointer()), blobHlsl->GetBufferSize() }
			);
		}

		//
		// Create pixel shader.
		//
		auto dxDev = this->dxDeviceSafeObj->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();

		// Finalize the the pixel shader graph.
		hr = this->dxPixelShaderGraph.shaderGraph->CreateModuleInstance(
			this->dxPixelShaderGraph.shaderGraphInstance.GetAddressOf(),
			nullptr
		);
		H::System::ThrowIfFailed(hr);

		// Link the pixel shader.
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		hr = this->dxPixelShaderGraph.linker->Link(
			this->dxPixelShaderGraph.shaderGraphInstance.Get(),
			"main",
			"ps_5_0",
			0,
			this->dxPixelShaderGraph.shaderBlob.GetAddressOf(),
			&errorBlob
		);
		if (errorBlob) {
			LOG_ERROR_D("d3dLinkerError = \"{}\"", (char*)errorBlob->GetBufferPointer());
		}
		H::System::ThrowIfFailed(hr);

		hr = d3dDev->CreatePixelShader(
			this->dxPixelShaderGraph.shaderBlob->GetBufferPointer(),
			this->dxPixelShaderGraph.shaderBlob->GetBufferSize(),
			nullptr,
			this->dxPixelShaderGraph.pixelShader.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);
	}


	Microsoft::WRL::ComPtr<ID3D11InputLayout> DxLinkageGraphPipeline::GetInputLayout() {
		return this->dxVertexShaderGraph.inputLayout;
	}

	Microsoft::WRL::ComPtr<ID3D11VertexShader> DxLinkageGraphPipeline::GetVertexShader() {
		return this->dxVertexShaderGraph.vertexShader;
	}
	
	Microsoft::WRL::ComPtr<ID3D11PixelShader> DxLinkageGraphPipeline::GetPixelShader() {
		return this->dxPixelShaderGraph.pixelShader;
	}



	DxShaderMudule DxLinkageGraphPipeline::LoadShaderModule(HlslModule hlslModule) {
		HRESULT hr = S_OK;
		DxShaderMudule dxShaderModule;

		auto hlslBlob = H::FS::ReadFile(hlslModule.hlslFile);

		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		hr = D3DCompile(
			hlslBlob.data(),
			hlslBlob.size(),
			("ShaderModule__" + hlslModule.hlslFile.filename().string()).c_str(),
			NULL,
			NULL,
			NULL,
			"lib_5_0",
			D3DCOMPILE_OPTIMIZATION_LEVEL3,
			0,
			dxShaderModule.compiledCodeBlob.GetAddressOf(),
			&errorBlob
		);
		if (errorBlob) {
			LOG_ERROR_D("d3dCompileError = \"{}\"", (char*)errorBlob->GetBufferPointer());
		}
		H::System::ThrowIfFailed(hr);

		// Load the compiled library code into a module object.
		hr = D3DLoadModule(
			dxShaderModule.compiledCodeBlob->GetBufferPointer(),
			dxShaderModule.compiledCodeBlob->GetBufferSize(),
			dxShaderModule.shaderLibrary.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		// Create an instance of the library and define resource bindings for it.
		// In this sample we use the same slots as the source library however this is not required.
		hr = dxShaderModule.shaderLibrary->CreateInstance(
			"",
			dxShaderModule.shaderLibraryInstance.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		if (hlslModule.bindResource) {
			dxShaderModule.shaderLibraryInstance->BindResource(
				hlslModule.bindResource->srcSlot,
				hlslModule.bindResource->dstSlot,
				hlslModule.bindResource->count
			);
		}
		if (hlslModule.bindSampler) {
			dxShaderModule.shaderLibraryInstance->BindSampler(
				hlslModule.bindSampler->srcSlot,
				hlslModule.bindSampler->dstSlot,
				hlslModule.bindSampler->count
			);
		}
		for (auto& bindConstantBuffer : hlslModule.bindConstantBuffers) {
			dxShaderModule.shaderLibraryInstance->BindConstantBuffer(
				bindConstantBuffer.srcSlot,
				bindConstantBuffer.dstSlot,
				bindConstantBuffer.cbDstOffset
			);
		}

		return dxShaderModule;
	}

	void DxLinkageGraphPipeline::PassValues(
		DxShaderGraph* dxShaderGraph,
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> prevNode,
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> currentNode,
		std::vector<HlslFunction::ParamMatch> paramsMatch)
	{
		HRESULT hr = S_OK;
		LOG_ASSERT(dxShaderGraph);

		for (auto& paramMatch : paramsMatch) {
			LOG_ASSERT(paramMatch.to != Param::_Return);
			hr = dxShaderGraph->shaderGraph->PassValue(prevNode.Get(), paramMatch.from, currentNode.Get(), paramMatch.to);
			H::System::ThrowIfFailed(hr);
		}
	}
}