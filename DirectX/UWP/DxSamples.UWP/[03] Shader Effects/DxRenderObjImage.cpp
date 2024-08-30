#include "DxRenderObjImage.h"
#include <Helpers/FileSystem.h>

const std::filesystem::path g_shaderLoadDir = H::ExePath() / L"" PP_STRINGIFY(MSBuildProject__RootNamespace);

namespace DxSamples {
	DxRenderObjImage::DxRenderObjImage(
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel,
		Params params)
		: swapChainPanel{ swapChainPanel }
	{
		this->dxRenderObjData = this->CreateObjectData(this->swapChainPanel->GetDxDevice(), params);
		this->CreateWindowSizeDependentResources();
	}

	void DxRenderObjImage::CreateWindowSizeDependentResources() {
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();

		auto outputSize = this->swapChainPanel->GetOutputSize();
		float outputAR = outputSize.width / outputSize.height;

		D3D11_TEXTURE2D_DESC texDesc;
		this->dxRenderObjData->texture->GetDesc(&texDesc);
		float texAR = float(texDesc.Width) / texDesc.Height;

		float scaleFactor = 1.0f;
		DirectX::XMStoreFloat4x4(
			&this->dxRenderObjData->vsConstantBufferData.mWorldViewProj,
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixScaling(
					(scaleFactor / outputAR) * texAR,
					scaleFactor,
					scaleFactor
				)
			)
		);
	}

	void DxRenderObjImage::ReleaseDeviceDependentResources() {
		this->dxRenderObjData->Reset();
	}

	void DxRenderObjImage::UpdateBuffers() {
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();
		auto d3dCtx = dxCtx->D3D();

		d3dCtx->UpdateSubresource(this->dxRenderObjData->vsConstantBuffer.Get(), 0, nullptr, &this->dxRenderObjData->vsConstantBufferData, 0, 0);
		d3dCtx->UpdateSubresource(this->dxRenderObjData->psConstantBuffer.Get(), 0, nullptr, &this->dxRenderObjData->psConstantBufferData, 0, 0);
	}

	std::unique_ptr<DxRenderObjImageData> DxRenderObjImage::CreateObjectData(
		H::Dx::DxDeviceSafeObj* dxDeviceSafeObj,
		Params params)
	{
		HRESULT hr = S_OK;

		auto dxDev = dxDeviceSafeObj->Lock();
		auto d3dDev = dxDev->GetD3DDevice();
		auto dxCtx = dxDev->LockContext();

		auto dxRenderObjData = std::make_unique<DxRenderObjImageData>();

		// Load vertex shader.
		{
			if (params.vertexShader.empty()) {
				params.vertexShader = g_shaderLoadDir / L"Default_VS.cso";
			}
			else {
				params.vertexShader = H::ExePath() / params.vertexShader;
			}

			auto vertexShaderBlob = H::FS::ReadFile(params.vertexShader);
			hr = d3dDev->CreateVertexShader(
				vertexShaderBlob.data(),
				vertexShaderBlob.size(),
				nullptr,
				dxRenderObjData->vertexShader.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);
		}

		// Load pixel shaders.
		{
			if (params.pixelShader.empty()) {
				params.pixelShader = g_shaderLoadDir / L"Default_PS.cso";
			}
			else {
				params.pixelShader = H::ExePath() / params.pixelShader;
			}

			auto pixelShaderHDRBlob = H::FS::ReadFile(params.pixelShader);
			hr = d3dDev->CreatePixelShader(
				pixelShaderHDRBlob.data(),
				pixelShaderHDRBlob.size(),
				nullptr,
				dxRenderObjData->pixelShader.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);
		}

		// VS constant buffer.
		{
			DirectX::XMStoreFloat4x4(
				&dxRenderObjData->vsConstantBufferData.mWorldViewProj,
				DirectX::XMMatrixTranspose(
					DirectX::XMMatrixIdentity()
				)
			);

			D3D11_BUFFER_DESC constantbufferDesc;
			constantbufferDesc.ByteWidth = sizeof(DxRenderObjImageData::VS_CONSTANT_BUFFER);
			constantbufferDesc.Usage = D3D11_USAGE_DEFAULT;
			constantbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			constantbufferDesc.CPUAccessFlags = 0;
			constantbufferDesc.MiscFlags = 0;
			constantbufferDesc.StructureByteStride = 0;

			hr = d3dDev->CreateBuffer(
				&constantbufferDesc,
				nullptr,
				dxRenderObjData->vsConstantBuffer.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);
		}

		// PS constant buffer.
		{
			// Store default values
			dxRenderObjData->psConstantBufferData.someData = { 0.0f, 0.0f, 0.0f, 0.0f };

			D3D11_BUFFER_DESC constantbufferDesc;
			constantbufferDesc.ByteWidth = sizeof(DxRenderObjImageData::PS_CONSTANT_BUFFER);
			constantbufferDesc.Usage = D3D11_USAGE_DEFAULT;
			constantbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			constantbufferDesc.CPUAccessFlags = 0;
			constantbufferDesc.MiscFlags = 0;
			constantbufferDesc.StructureByteStride = 0;

			hr = d3dDev->CreateBuffer(
				&constantbufferDesc,
				nullptr,
				dxRenderObjData->psConstantBuffer.ReleaseAndGetAddressOf()
			);
			H::System::ThrowIfFailed(hr);
		}

		// Create texture.
		{
			D3D11_TEXTURE2D_DESC txtDesc = {};
			txtDesc.MipLevels = txtDesc.ArraySize = 1;
			//txtDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			txtDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			txtDesc.SampleDesc.Count = 1;
			txtDesc.Usage = D3D11_USAGE_IMMUTABLE;
			txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

			auto imageBlob = H::Dx::Tools::LoadBGRAImage(params.imagePath, txtDesc.Width, txtDesc.Height);

			D3D11_SUBRESOURCE_DATA initialData = {};
			initialData.pSysMem = imageBlob.data();
			initialData.SysMemPitch = txtDesc.Width * sizeof(uint32_t);

			hr = d3dDev->CreateTexture2D(&txtDesc, &initialData, dxRenderObjData->texture.GetAddressOf());
			H::System::ThrowIfFailed(hr);

			hr = d3dDev->CreateShaderResourceView(dxRenderObjData->texture.Get(), nullptr, dxRenderObjData->textureSRV.ReleaseAndGetAddressOf());
			H::System::ThrowIfFailed(hr);
		}

		// Create sampler.
		{
			dxRenderObjData->sampler = H::Dx::CreateLinearSampler(d3dDev);
		}

		return dxRenderObjData;
	}
}