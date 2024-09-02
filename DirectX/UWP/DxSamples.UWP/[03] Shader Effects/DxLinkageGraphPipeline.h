#pragma once
#include <Helpers/Dx/DxDevice.h>
#include <d3dcompiler.h>

namespace DxSamples {
	struct DxShaderMudule {
		Microsoft::WRL::ComPtr<ID3DBlob> compiledCodeBlob; // save compiled shader code from .hlsl because it must be available when ID3DLinker->Link called.
		Microsoft::WRL::ComPtr<ID3D11Module> shaderLibrary;
		Microsoft::WRL::ComPtr<ID3D11ModuleInstance> shaderLibraryInstance;
		std::vector<Microsoft::WRL::ComPtr<ID3D11LinkingNode>> shaderCallFunctionNodes;
	};

	struct DxShaderGraph {
		enum class Type {
			Vertex,
			Pixel,
		};

		Type type;
		Microsoft::WRL::ComPtr<ID3D11Linker> linker;
		Microsoft::WRL::ComPtr<ID3D11FunctionLinkingGraph> shaderGraph;
		Microsoft::WRL::ComPtr<ID3D11ModuleInstance> shaderGraphInstance;
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> shaderInputNode;
		Microsoft::WRL::ComPtr<ID3D11LinkingNode> shaderOutputNode;
		Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
		std::vector<DxShaderMudule> dxShaderModules;

	protected:
		DxShaderGraph(Type type)
			: type{ type }
		{
			HRESULT hr = S_OK;

			hr = D3DCreateLinker(this->linker.GetAddressOf());
			H::System::ThrowIfFailed(hr);

			hr = D3DCreateFunctionLinkingGraph(0, this->shaderGraph.GetAddressOf());
			H::System::ThrowIfFailed(hr);
		}
	};

	struct DxVertexShaderGraph : DxShaderGraph {
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;

		DxVertexShaderGraph()
			: DxShaderGraph{ DxShaderGraph::Type::Vertex }
		{}
	};

	struct DxPixelShaderGraph : DxShaderGraph {
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

		DxPixelShaderGraph()
			: DxShaderGraph{ DxShaderGraph::Type::Pixel }
		{}
	};



	enum Param {
		_Return = D3D_RETURN_PARAMETER_INDEX,
		_0 = 0,
		_1,
		_2,
		_3,
	};

	struct HlslFunction {
		struct ParamMatch {
			Param from;
			Param to;
		};
		std::string functionName;
		std::vector<ParamMatch> paramsMatch;
	};

	struct HlslModule {
		struct BindResource {
			uint32_t srcSlot = 0;
			uint32_t dstSlot = 0;
			uint32_t count = 0;
		};
		struct BindSampler {
			uint32_t srcSlot = 0;
			uint32_t dstSlot = 0;
			uint32_t count = 0;
		};
		struct BindConstantBuffer {
			uint32_t srcSlot = 0;
			uint32_t dstSlot = 0;
			uint32_t cbDstOffset = 0;
		};

		std::filesystem::path hlslFile;
		std::vector<HlslFunction> hlslFunctions;
		std::optional<BindResource> bindResource;
		std::optional<BindSampler> bindSampler;
		std::vector<BindConstantBuffer> bindConstantBuffers;
	};

	struct DxVertexShaderGraphDesc {
		std::vector<D3D11_INPUT_ELEMENT_DESC> vertexInputLayout;
		std::vector<D3D11_PARAMETER_DESC> shaderInputParameters;
		std::vector<D3D11_PARAMETER_DESC> shaderOutputParameters;
		std::vector<HlslModule> hlslModules;
	};

	struct DxPixelShaderGraphDesc {
		std::vector<D3D11_PARAMETER_DESC> shaderInputParameters;
		std::vector<D3D11_PARAMETER_DESC> shaderOutputParameters;
		std::vector<HlslModule> hlslModules;
	};

	class DxLinkageGraphPipeline {
	public:

		DxLinkageGraphPipeline(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj);

		void CreateVertexShaderFromGraphDesc(const DxVertexShaderGraphDesc& dxVertexShaderGraphDesc);
		void CreatePixelShaderFromGraphDesc(const DxPixelShaderGraphDesc& dxPixelShaderGraphDesc);

		Microsoft::WRL::ComPtr<ID3D11InputLayout> GetInputLayout();
		Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();
		Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();

	private:
		DxShaderMudule LoadShaderModule(HlslModule hlslModule);

		void PassValues(
			DxShaderGraph* dxShaderGraph,
			Microsoft::WRL::ComPtr<ID3D11LinkingNode> prevNode,
			Microsoft::WRL::ComPtr<ID3D11LinkingNode> currentNode,
			std::vector<HlslFunction::ParamMatch> paramsMatch
		);

	private:
		H::Dx::DxDeviceSafeObj* dxDeviceSafeObj;
		DxVertexShaderGraph dxVertexShaderGraph;
		DxPixelShaderGraph dxPixelShaderGraph;
	};
}