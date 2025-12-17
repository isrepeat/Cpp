#pragma once
#include "Helpers/common.h"
#include "HlslModule.h"
#include "DxDevice.h"
#include <d3dcompiler.h>

namespace HELPERS_NS {
	namespace Dx {
		namespace details {
			struct DxShaderMudule {
				Microsoft::WRL::ComPtr<ID3DBlob> compiledCodeBlob; // save compiled shader code from .hlsl because it must be available when ID3DLinker->Link called.
				Microsoft::WRL::ComPtr<ID3D11Module> shaderLibrary;
				Microsoft::WRL::ComPtr<ID3D11ModuleInstance> shaderLibraryInstance;

				std::vector<HlslModule::BindConstantBuffer> shaderBindedConstantBuffers;
				std::vector<Microsoft::WRL::ComPtr<ID3D11LinkingNode>> shaderCallFunctionNodes;
			};

			struct DxShaderGraph {
				enum class Type {
					Vertex,
					Pixel,
				};

				const Type type;
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
					HELPERS_NS::System::ThrowIfFailed(hr);

					hr = D3DCreateFunctionLinkingGraph(0, this->shaderGraph.GetAddressOf());
					HELPERS_NS::System::ThrowIfFailed(hr);
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
		}
	}
}