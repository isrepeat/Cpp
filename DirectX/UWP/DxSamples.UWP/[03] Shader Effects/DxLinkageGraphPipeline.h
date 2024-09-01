#pragma once
#include <Helpers/Dx/DxDevice.h>
#include <d3dcompiler.h>

namespace DxSamples {
	struct DxShaderMudule {
		Microsoft::WRL::ComPtr<ID3DBlob> compiledCodeBlob; // save compiled shader code from .hlsl because it must be available when ID3DLinker->Link called.
		Microsoft::WRL::ComPtr<ID3D11Module> shaderLibrary;
		Microsoft::WRL::ComPtr<ID3D11ModuleInstance> shaderLibraryInstance;
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> shaderCallFunctionNode; // TODO: add support multifunctions linking.
	};


	struct HlslModule {
		std::filesystem::path hlslFile;
		std::vector<std::string> callFunctions;
	};

	// Vertex Shader Function Linking Graph
	struct DxVertexShaderFLG {
		std::vector<D3D11_INPUT_ELEMENT_DESC> vertexInputLayout;
		std::vector<D3D11_PARAMETER_DESC> shaderInputParameters;
		std::vector<D3D11_PARAMETER_DESC> shaderOutputParameters;
		std::vector<HlslModule> hlslModules;
	};

	struct DxPixelShaderFLG {
		std::vector<D3D11_PARAMETER_DESC> shaderInputParameters;
		std::vector<D3D11_PARAMETER_DESC> shaderOutputParameters;
		std::vector<HlslModule> hlslModules;
	};

	class DxLinkageGraphPipeline {
	public:
		DxLinkageGraphPipeline(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj);

		void AddVertexShaderFLG(const DxVertexShaderFLG& dxVertexShaderFLG);
		void AddPixelShaderFLG(const DxPixelShaderFLG& dxPixelShaderFLG);
		
		//void SetInputSignature();

		//DxShaderMudule AddModule(
		//	const std::filesystem::path& hlslFile,
		//	const std::string& callFunctionName);

		//void CallFunction(DxShaderMudule& dxShaderModule, const std::string& functionName);
		//void SetOutputSignature();
		//void LogVertexShaderGraph();
		//Microsoft::WRL::ComPtr<ID3D11VertexShader> CreateVertexShader();

		Microsoft::WRL::ComPtr<ID3D11InputLayout> GetInputLayout();
		Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();
		Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();

	private:
		H::Dx::DxDeviceSafeObj* dxDeviceSafeObj;

		Microsoft::WRL::ComPtr<ID3D11Linker> vertexLinker;
		Microsoft::WRL::ComPtr<ID3D11FunctionLinkingGraph> vertexShaderGraph;
		Microsoft::WRL::ComPtr<ID3D11ModuleInstance> vertexShaderGraphInstance;
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> vertexShaderInputNode;
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> vertexShaderOutputNode;
		Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		std::vector<DxShaderMudule> dxVertexShaderModules;

		
		Microsoft::WRL::ComPtr<ID3D11Linker> pixelLinker;
		Microsoft::WRL::ComPtr<ID3D11FunctionLinkingGraph> pixelShaderGraph;
		Microsoft::WRL::ComPtr<ID3D11ModuleInstance> pixelShaderGraphInstance;
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> pixelShaderInputNode;
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> pixelShaderOutputNode;
		Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		std::vector<DxShaderMudule> dxPixelShaderModules;
	};
}