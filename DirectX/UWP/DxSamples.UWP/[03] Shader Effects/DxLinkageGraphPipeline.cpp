#include "DxLinkageGraphPipeline.h"
#include <Helpers/FileSystem.h>


namespace DxSamples {
	DxLinkageGraphPipeline::DxLinkageGraphPipeline(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj)
		: dxDeviceSafeObj{ dxDeviceSafeObj }
	{
		HRESULT hr = S_OK;

		hr = D3DCreateLinker(this->linker.GetAddressOf());
		H::System::ThrowIfFailed(hr);

		hr = D3DCreateFunctionLinkingGraph(0, this->vertexShaderGraph.GetAddressOf());
		H::System::ThrowIfFailed(hr);
	}


	DxShaderMudule DxLinkageGraphPipeline::AddModule(
		const std::filesystem::path& hlslFile)
		//const std::string& callFunctionName)
	{
		HRESULT hr = S_OK;

		auto hlslBlob = H::FS::ReadFile(hlslFile);

		Microsoft::WRL::ComPtr<ID3DBlob> codeBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		hr = D3DCompile(
			hlslBlob.data(),
			hlslBlob.size(),
			("ShaderModule__" + hlslFile.filename().string()).c_str(),
			NULL,
			NULL,
			NULL,
			"lib_5_0",
			D3DCOMPILE_OPTIMIZATION_LEVEL3,
			0,
			&codeBlob,
			&errorBlob
		);

		std::string d3dCompileError;
		if (errorBlob) {
			d3dCompileError = (char*)errorBlob->GetBufferPointer();
		}
		H::System::ThrowIfFailed(hr);


		// Load the compiled library code into a module object.
		Microsoft::WRL::ComPtr<ID3D11Module> shaderLibrary;
		hr = D3DLoadModule(
			codeBlob->GetBufferPointer(),
			codeBlob->GetBufferSize(),
			shaderLibrary.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		// Create an instance of the library and define resource bindings for it.
		// In this sample we use the same slots as the source library however this is not required.
		Microsoft::WRL::ComPtr<ID3D11ModuleInstance> shaderLibraryInstance;
		hr = shaderLibrary->CreateInstance("", shaderLibraryInstance.GetAddressOf());
		H::System::ThrowIfFailed(hr);

		// Hook up the shader library instance.
		hr = this->linker->UseLibrary(shaderLibraryInstance.Get());
		H::System::ThrowIfFailed(hr);

		this->dxShaderModules.push_back(DxShaderMudule{
			shaderLibrary,
			shaderLibraryInstance,
			{}
			});

		return this->dxShaderModules.back();
	}


	//void DxLinkageGraphPipeline::SetInputOutputSignature() {
	//	HRESULT hr = S_OK;

	//	// Define the vertex shader input layout.
	//	static const D3D11_PARAMETER_DESC vertexShaderInputParameters[] =
	//	{
	//		{ "inputPosition", "SV_POSITION", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1,4,D3D_INTERPOLATION_LINEAR, D3D_PF_IN,0,0,0,0 },
	//		{ "inputUV", "TEXCOORD0", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1,2,D3D_INTERPOLATION_LINEAR,D3D_PF_IN,0,0,0,0}
	//	};

	//	hr = this->vertexShaderGraph->SetInputSignature(
	//		vertexShaderInputParameters,
	//		ARRAYSIZE(vertexShaderInputParameters),
	//		this->vertexShaderInputNode.GetAddressOf()
	//	);
	//	H::System::ThrowIfFailed(hr);

	//	// Define the output layout for the vertex function.
	//	static const D3D11_PARAMETER_DESC vertexShaderOutputParameters[] =
	//	{
	//		{ "outputPosition", "SV_POSITION", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1,4,D3D_INTERPOLATION_UNDEFINED, D3D_PF_OUT, 0,0,0,0},
	//		{ "outputUV","TEXCOORD0",D3D_SVT_FLOAT, D3D_SVC_VECTOR,1,2,D3D_INTERPOLATION_UNDEFINED,D3D_PF_OUT,0,0,0,0 },
	//	};

	//	// Create the vertex output node.
	//	hr = this->vertexShaderGraph->SetOutputSignature(
	//		vertexShaderOutputParameters,
	//		ARRAYSIZE(vertexShaderOutputParameters),
	//		this->vertexShaderOutputNode.GetAddressOf()
	//	);
	//	H::System::ThrowIfFailed(hr);
	//}

	void DxLinkageGraphPipeline::SetInputSignature() {
		HRESULT hr = S_OK;

		// Define the vertex shader input layout.
		static const D3D11_PARAMETER_DESC vertexShaderInputParameters[] =
		{
			{ "inputPosition", "SV_POSITION", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1,4,D3D_INTERPOLATION_LINEAR, D3D_PF_IN,0,0,0,0 },
			{ "inputUV", "TEXCOORD0", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1,2,D3D_INTERPOLATION_LINEAR,D3D_PF_IN,0,0,0,0}
		};

		hr = this->vertexShaderGraph->SetInputSignature(
			vertexShaderInputParameters,
			ARRAYSIZE(vertexShaderInputParameters),
			this->vertexShaderInputNode.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);
	}

	void DxLinkageGraphPipeline::CallFunction(DxShaderMudule& dxShaderModule, const std::string& functionName) {
		HRESULT hr = S_OK;

		// Create a node for the main VertexFunction call using the output of the helper functions.
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> shaderCallFunctionNode;
		hr = this->vertexShaderGraph->CallFunction(
			"",
			dxShaderModule.shaderLibrary.Get(),
			functionName.c_str(),
			dxShaderModule.shaderCallFunctionNode.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);
	}

	void DxLinkageGraphPipeline::SetOutputSignature() {
		HRESULT hr = S_OK;

		// Define the output layout for the vertex function.
		static const D3D11_PARAMETER_DESC vertexShaderOutputParameters[] =
		{
			{ "outputPosition", "SV_POSITION", D3D_SVT_FLOAT, D3D_SVC_VECTOR, 1,4,D3D_INTERPOLATION_UNDEFINED, D3D_PF_OUT, 0,0,0,0},
			{ "outputUV","TEXCOORD0",D3D_SVT_FLOAT, D3D_SVC_VECTOR,1,2,D3D_INTERPOLATION_UNDEFINED,D3D_PF_OUT,0,0,0,0 },
		};

		// Create the vertex output node.
		hr = this->vertexShaderGraph->SetOutputSignature(
			vertexShaderOutputParameters,
			ARRAYSIZE(vertexShaderOutputParameters),
			this->vertexShaderOutputNode.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);
	}


	Microsoft::WRL::ComPtr<ID3D11FunctionLinkingGraph> DxLinkageGraphPipeline::GetVertexShaderGraph() {
		return this->vertexShaderGraph;
	}

	Microsoft::WRL::ComPtr<ID3D11LinkingNode> DxLinkageGraphPipeline::GetVertexShaderInputNode() {
		return this->vertexShaderInputNode;
	}

	Microsoft::WRL::ComPtr<ID3D11LinkingNode> DxLinkageGraphPipeline::GetVertexShaderOutputNode() {
		return this->vertexShaderOutputNode;
	}


	void DxLinkageGraphPipeline::LogVertexShaderGraph() {
		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<ID3DBlob> blobHlsl;
		hr = this->vertexShaderGraph->GenerateHlsl(0, &blobHlsl);
		H::System::ThrowIfFailed(hr);

		LOG_DEBUG_D("vertexShaderGraph --> hlsl:\n"
			"{}"
			, std::string{ static_cast<const char*>(blobHlsl->GetBufferPointer()), blobHlsl->GetBufferSize() }
		);
	}


	Microsoft::WRL::ComPtr<ID3D11VertexShader> DxLinkageGraphPipeline::CreateVertexShader() {
		auto dxDev = this->dxDeviceSafeObj->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();

		HRESULT hr = S_OK;

		// Finalize the the vertex shader graph.
		hr = this->vertexShaderGraph->CreateModuleInstance(
			this->vertexShaderGraphInstance.GetAddressOf(),
			nullptr
		);
		H::System::ThrowIfFailed(hr);

		// Link the vertex shader.
		ID3DBlob* vertexShaderBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		hr = this->linker->Link(
			this->vertexShaderGraphInstance.Get(),
			"main",
			"vs_5_0",
			0,
			&vertexShaderBlob,
			&errorBlob
		);
		H::System::ThrowIfFailed(hr);

		static const D3D11_INPUT_ELEMENT_DESC inputElementDesc[2] = {
			{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		hr = d3dDev->CreateInputLayout(
			inputElementDesc,
			ARRAYSIZE(inputElementDesc),
			vertexShaderBlob->GetBufferPointer(),
			vertexShaderBlob->GetBufferSize(),
			this->inputLayout.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		hr = d3dDev->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),
			vertexShaderBlob->GetBufferSize(),
			nullptr,
			this->vertexShader.GetAddressOf()
		);
		H::System::ThrowIfFailed(hr);

		return this->vertexShader;
	}


	Microsoft::WRL::ComPtr<ID3D11VertexShader> DxLinkageGraphPipeline::GetVertexShader() {
		return this->vertexShader;
	}

	Microsoft::WRL::ComPtr<ID3D11InputLayout> DxLinkageGraphPipeline::GetInputLayout() {
		return this->inputLayout;
	}
}