#pragma once
#include <Helpers/Dx/DxDevice.h>
#include <d3dcompiler.h>

namespace DxSamples {
	struct DxShaderMudule {
		Microsoft::WRL::ComPtr<ID3DBlob> compiledCodeBlob; // save loaded .hlsl compiledCode [blob] for ID3DLinker->Link 
		Microsoft::WRL::ComPtr<ID3D11Module> shaderLibrary;
		Microsoft::WRL::ComPtr<ID3D11ModuleInstance> shaderLibraryInstance;
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> shaderCallFunctionNode;
	};

	class DxLinkageGraphPipeline {
	public:
		DxLinkageGraphPipeline(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj);
		
		Microsoft::WRL::ComPtr<ID3D11FunctionLinkingGraph> GetVertexShaderGraph();
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> GetVertexShaderInputNode();
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> GetVertexShaderOutputNode();
		Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();
		Microsoft::WRL::ComPtr<ID3D11InputLayout> GetInputLayout();

		DxShaderMudule AddModule(
			const std::filesystem::path& hlslFile);
			//const std::string& callFunctionName);

		void SetInputSignature();
		
		void CallFunction(DxShaderMudule& dxShaderModule, const std::string& functionName);
		
		void SetOutputSignature();

		void LogVertexShaderGraph();

		Microsoft::WRL::ComPtr<ID3D11VertexShader> CreateVertexShader();

	private:
		H::Dx::DxDeviceSafeObj* dxDeviceSafeObj;
		Microsoft::WRL::ComPtr<ID3D11Linker> linker;
		Microsoft::WRL::ComPtr<ID3D11FunctionLinkingGraph> vertexShaderGraph;
		Microsoft::WRL::ComPtr<ID3D11ModuleInstance> vertexShaderGraphInstance;

		std::vector<DxShaderMudule> dxShaderModules;
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> vertexShaderInputNode;
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> vertexShaderOutputNode;

		Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	};
}