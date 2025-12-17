#pragma once
#include "Helpers/common.h"
#include "DxShaderModule.h"
#include "HlslModule.h"
#include "DxDevice.h"

#include <d3dcompiler.h>
#include <memory>

namespace HELPERS_NS {
	namespace Dx {
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

		class DxLinkingGraph {
		public:
			DxLinkingGraph(HELPERS_NS::Dx::DxDeviceSafeObj* dxDeviceSafeObj);

			void CreateVertexShaderFromGraphDesc(const DxVertexShaderGraphDesc& dxVertexShaderGraphDesc);
			void CreatePixelShaderFromGraphDesc(const DxPixelShaderGraphDesc& dxPixelShaderGraphDesc);

			void UpdateConstantBuffers();
			void SetShadersToContext();

			Microsoft::WRL::ComPtr<ID3D11InputLayout> GetInputLayout();
			Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader();
			Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();

		private:
			details::DxShaderMudule LoadShaderModule(HlslModule hlslModule);

			void PassValues(
				details::DxShaderGraph* dxShaderGraph,
				Microsoft::WRL::ComPtr<ID3D11LinkingNode> prevNode,
				Microsoft::WRL::ComPtr<ID3D11LinkingNode> currentNode,
				std::vector<HlslFunction::ParamMatch> paramsMatch
			);

		private:
			HELPERS_NS::Dx::DxDeviceSafeObj* dxDeviceSafeObj;
			details::DxVertexShaderGraph dxVertexShaderGraph;
			details::DxPixelShaderGraph dxPixelShaderGraph;
		};
	}
}