#include "RenderPipeline.h"
#include <Helpers/Dx/Shaders/ShadersCommon.h>
#include <Helpers/FileSystem.h>

namespace HELPERS_NS {
	namespace Dx {
		RenderPipeline::RenderPipeline(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel)
			: swapChainPanel{ swapChainPanel }
		{
			LOG_ASSERT(swapChainPanel);
			HRESULT hr = S_OK;

			auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
			auto d3dDev = dxDev->GetD3DDevice();
			auto dxCtx = dxDev->LockContext();
			auto d2dCtx = dxCtx->D2D();

			// Create default render object.
			auto& dxRenderObjData = this->quadGeometryData;

			// Create vertex buffer.
			{
				float w = 1.0f;
				float h = 1.0f;
				static const VertexPositionTexcoord vertexData[4] = {
					{ { -w, -h, 0.1f, 1.0f }, { 0.0f, 1.0f } }, // LB
					{ { -w,  h, 0.1f, 1.0f }, { 0.0f, 0.0f } }, // LT
					{ {  w,  h, 0.1f, 1.0f }, { 1.0f, 0.0f } }, // RT
					{ {  w, -h, 0.1f, 1.0f }, { 1.0f, 1.0f } }, // RB
				};

				D3D11_SUBRESOURCE_DATA initData = {};
				initData.pSysMem = vertexData;

				D3D11_BUFFER_DESC vertexBufferDesc = {};
				vertexBufferDesc.ByteWidth = sizeof(vertexData);
				vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
				vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				vertexBufferDesc.StructureByteStride = sizeof(VertexPositionTexcoord);

				hr = d3dDev->CreateBuffer(
					&vertexBufferDesc,
					&initData,
					dxRenderObjData.vertexBuffer.ReleaseAndGetAddressOf()
				);
				H::System::ThrowIfFailed(hr);
			}

			// Create index buffer.
			{
				static const uint16_t indexData[6] = {
					0,1,2,
					2,3,0,
				};

				D3D11_SUBRESOURCE_DATA initData = {};
				initData.pSysMem = indexData;

				D3D11_BUFFER_DESC indexBufferDesc = {};
				indexBufferDesc.ByteWidth = sizeof(indexData);
				indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
				indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				indexBufferDesc.StructureByteStride = sizeof(uint16_t);

				hr = d3dDev->CreateBuffer(
					&indexBufferDesc,
					&initData,
					dxRenderObjData.indexBuffer.ReleaseAndGetAddressOf()
				);
				H::System::ThrowIfFailed(hr);
			}

			// Set default sampler.
			{
				this->sampler = H::Dx::CreateLinearSampler(d3dDev);
			}

			// Load vertex shader and create input layout.
			{
				auto vertexShaderBlob = H::FS::ReadFile(g_shaderLoadDir / L"defaultVS.cso");
				hr = d3dDev->CreateVertexShader(
					vertexShaderBlob.data(),
					vertexShaderBlob.size(),
					nullptr,
					dxRenderObjData.vertexShader.ReleaseAndGetAddressOf()
				);
				H::System::ThrowIfFailed(hr);

				static const D3D11_INPUT_ELEMENT_DESC inputElementDesc[2] = {
					{ "SV_Position", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,  0 },
					{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA , 0 },
				};

				hr = d3dDev->CreateInputLayout(
					inputElementDesc,
					_countof(inputElementDesc),
					vertexShaderBlob.data(),
					vertexShaderBlob.size(),
					dxRenderObjData.inputLayout.ReleaseAndGetAddressOf()
				);
				H::System::ThrowIfFailed(hr);

				// Set default vertex shader.
				this->vertexShader = dxRenderObjData.vertexShader;
				this->inputLayout = dxRenderObjData.inputLayout;
			}

			this->swapChainPanel->GetNotifications()->onPresent.Subscribe([this] {
				this->ClearState();
				});
		}

		RenderPipeline::RenderPipeline(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj)
			: RenderPipeline(dxDeviceSafeObj->Lock()->GetAssociatedSwapChainPanel())
		{
		}

		void RenderPipeline::SetTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV) {
			this->textureSRV = textureSRV;
		}

		void RenderPipeline::SetSampler(Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler) {
			this->sampler = sampler;
		}

		//void RenderPipeline::AddPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader) {
		//	this->pixelShaders.push_back(pixelShader);
		//}


		void RenderPipeline::SetInputLayout(Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout) {
			this->inputLayout = inputLayout;
		}

		void RenderPipeline::SetVertexShader(
			Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader,
			Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer) 
		{
			this->vertexShader = vertexShader;
			this->vsConstantBuffer = vsConstantBuffer;
		}

		void RenderPipeline::SetPixelShader(
			Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader,
			Microsoft::WRL::ComPtr<ID3D11Buffer> psConstantBuffer)
		{
			this->pixelShader = pixelShader;
			this->psConstantBuffer = psConstantBuffer;
		}

		void RenderPipeline::SetLinkingGraph(std::shared_ptr<H::Dx::DxLinkingGraph> dxLinkingGraph) {
			this->dxLinkingGraph = dxLinkingGraph;
		}

		// Draw passed render obj geometry as quad with custom state (shaders, constants buffers, ...)
		void RenderPipeline::Draw() {
			auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
			auto d3dDevice = dxDev->GetD3DDevice();
			auto dxCtx = dxDev->LockContext();
			auto d3dCtx = dxCtx->D3D();

			if (this->dxLinkingGraph) {
				this->dxLinkingGraph->UpdateConstantBuffers();

				// Set texture and sampler.
				auto pTextureSRV = this->textureSRV.Get();
				d3dCtx->PSSetShaderResources(0, 1, &pTextureSRV);

				auto pSampler = this->sampler.Get();
				d3dCtx->PSSetSamplers(0, 1, &pSampler);

				// Set vetex / pixel shaders.
				this->dxLinkingGraph->SetShadersToContext();

				// Set vertex / index buffers and input layout.
				UINT strides = sizeof(VertexPositionTexcoord);
				UINT offsets = 0;
				d3dCtx->IASetVertexBuffers(0, 1, this->quadGeometryData.vertexBuffer.GetAddressOf(), &strides, &offsets);
				d3dCtx->IASetIndexBuffer(this->quadGeometryData.indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

				d3dCtx->IASetInputLayout(this->dxLinkingGraph->GetInputLayout().Get());
				d3dCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				// Draw quad.
				d3dCtx->DrawIndexed(6, 0, 0);
			}
			else {
				//// Update constant buffer data
				//d3dCtx->UpdateSubresource(dxRenderObjBase->vsConstantBuffer.Get(), 0, nullptr, &dxRenderObjBase->vsConstantBufferData, 0, 0);

				// Set texture and sampler.
				auto pTextureSRV = this->textureSRV.Get();
				d3dCtx->PSSetShaderResources(0, 1, &pTextureSRV);

				auto pSampler = this->sampler.Get();
				d3dCtx->PSSetSamplers(0, 1, &pSampler);

				// Set vetex / pixel shaders.
				d3dCtx->VSSetShader(this->vertexShader.Get(), nullptr, 0);
				if (this->vsConstantBuffer) {
					d3dCtx->VSSetConstantBuffers(0, 1, this->vsConstantBuffer.GetAddressOf());
				}

				d3dCtx->PSSetShader(this->pixelShader.Get(), nullptr, 0);
				if (this->psConstantBuffer) {
					d3dCtx->PSSetConstantBuffers(0, 1, this->psConstantBuffer.GetAddressOf());
				}

				// Set vertex / index buffers and input layout.
				UINT strides = sizeof(VertexPositionTexcoord);
				UINT offsets = 0;
				d3dCtx->IASetVertexBuffers(0, 1, this->quadGeometryData.vertexBuffer.GetAddressOf(), &strides, &offsets);
				d3dCtx->IASetIndexBuffer(this->quadGeometryData.indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

				//d3dCtx->IASetInputLayout(this->quadGeometryData.inputLayout.Get());
				d3dCtx->IASetInputLayout(this->inputLayout.Get());
				d3dCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				// Draw quad.
				d3dCtx->DrawIndexed(6, 0, 0);
			}
		}


		void RenderPipeline::ClearState() {
			//this->pixelShaders.clear();
		}
	}
}