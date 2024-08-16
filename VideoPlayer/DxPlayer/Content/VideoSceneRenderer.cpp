#include "pch.h"
#if ENGINE_TYPE == MEDIA_ENDIGNE_LOGIC
#include "VideoSceneRenderer.h"
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/File.h>

#pragma comment(lib, "dxguid.lib")

namespace DxTools {
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> CreateFrameBitmap(
		H::Dx::DxDeviceCtxSafeObj_t::_Locked& dxCtx,
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture)
	{
		Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
		HRESULT hr = texture.As(&dxgiSurface);
		if (FAILED(hr))
			return nullptr;

		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		D2D1_BITMAP_PROPERTIES1 bitmapProperties =
			D2D1::BitmapProperties1(
				D2D1_BITMAP_OPTIONS_NONE,
				D2D1::PixelFormat(desc.Format, D2D1_ALPHA_MODE_IGNORE)
			);

		Microsoft::WRL::ComPtr<ID2D1Bitmap1> pBitmap;
		hr = dxCtx->D2D()->CreateBitmapFromDxgiSurface(
			dxgiSurface.Get(),
			&bitmapProperties,
			&pBitmap
		);

		if (FAILED(hr))
			return nullptr;

		return pBitmap;
	}
}

namespace DxPlayer {
	VideoSceneRenderer::VideoSceneRenderer(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel)
		: swapChainPanel{ swapChainPanel }
		, monitorRefreshRate{ H::Dx::GetRefreshRateForDXGIOutput(H::Dx::EnumOutputsState{ H::Dx::EnumAdaptersState{}.Next() }.Next().GetDXGIOutput()) }
		, monitorRefreshDuration{ monitorRefreshRate.Inversed().As<H::Chrono::seconds_f>() }
		, loadingComplete{ false }
	{
		HRESULT hr = this->swapChainPanel->GetDxDevice()->Lock()->GetD2DFactory()->CreateDrawingStateBlock(&this->stateBlock);
		H::System::ThrowIfFailed(hr);

		this->CreateDeviceDependentResources();
		this->CreateWindowSizeDependentResources();
	}

	VideoSceneRenderer::~VideoSceneRenderer()
	{}

	void VideoSceneRenderer::CreateDeviceDependentResources() {
		// Load shaders asynchronously.
		auto loadVSTask = H::WinRt::ReadDataAsync(L"Default_VS.cso");
		auto loadPSTask = H::WinRt::ReadDataAsync(L"Default_PS.cso");

		// After the vertex shader file is loaded, create the shader and input layout.
		auto createVSTask = loadVSTask.then([this](const std::vector<BYTE>& fileData) {
			auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();

			HRESULT hr = S_OK;
			hr = dxDev->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&this->vertexShader
			);
			H::System::ThrowIfFailed(hr);

			static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			hr = dxDev->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&this->inputLayout
			);
			H::System::ThrowIfFailed(hr);
		});

		// After the pixel shader file is loaded, create the shader and constant buffer.
		auto createPSTask = loadPSTask.then([this](const std::vector<BYTE>& fileData) {
			auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();

			HRESULT hr = S_OK;
			hr = dxDev->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&this->pixelShader
			);
			H::System::ThrowIfFailed(hr);

			CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			hr = dxDev->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&this->constantBuffer
			);
			H::System::ThrowIfFailed(hr);
			});

		// Once both shaders are loaded, create the mesh.
		(createPSTask && createVSTask).then([this] {
			this->loadingComplete = true;
			});

		HRESULT hr = S_OK;
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();

		// Create frame transformation effects
		hr = d2dCtx->CreateEffect(CLSID_D2D1Scale, &this->scaleEffect);
		H::System::ThrowIfFailed(hr);

		hr = d2dCtx->CreateEffect(CLSID_D2D12DAffineTransform, &this->centerEffect);
		H::System::ThrowIfFailed(hr);
	}

	void VideoSceneRenderer::CreateWindowSizeDependentResources() {
	}

	void VideoSceneRenderer::ReleaseDeviceDependentResources() {
		this->loadingComplete = false;
		this->vertexShader.Reset();
		this->inputLayout.Reset();
		this->pixelShader.Reset();
		this->constantBuffer.Reset();
		//this->vertexBuffer.Reset();
		//this->indexBuffer.Reset();
	}


	void VideoSceneRenderer::Init(std::unique_ptr<AvReader> avReader) {
		this->avReader = std::move(avReader);
	}

	void VideoSceneRenderer::Render() {
		// Loading is asynchronous. Only draw geometry after it's loaded.
		if (!this->loadingComplete) {
			return;
		}

		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();

		H::Size_f logicalSize = this->swapChainPanel->GetLogicalSize();

		d2dCtx->SaveDrawingState(this->stateBlock.Get());
		d2dCtx->BeginDraw();
		d2dCtx->Clear(D2D1::ColorF(D2D1::ColorF::Gray));


		std::unique_ptr<MF::MFVideoSample> videoSample;
		if (this->lastRenderedVideoSample) {
			H::Chrono::Hns elapsedTimeFromLastRenderedVideoSample = std::chrono::duration_cast<H::Chrono::Hns::_MyBase>(
				std::chrono::high_resolution_clock::now() - this->lastRenderedVideoSampleTimePoint);

			auto distanceToNextVideoSample = this->lastRenderedVideoSample->duration - elapsedTimeFromLastRenderedVideoSample;
			auto distanceToNextVideoSampleMs = distanceToNextVideoSample.ToDuration<H::Chrono::milliseconds_f>();

			bool elapsedTimeGreaterLastFrameDuration = elapsedTimeFromLastRenderedVideoSample > this->lastRenderedVideoSample->duration;
			bool nextVideoSampleNearCurrentVSyncPts = std::abs(distanceToNextVideoSampleMs.count()) < this->monitorRefreshDuration.count() / 4;

			bool shouldPresentNextSampleInThisIteration = elapsedTimeGreaterLastFrameDuration || nextVideoSampleNearCurrentVSyncPts;

			if (shouldPresentNextSampleInThisIteration) {
				this->lastRenderedVideoSampleTimePoint = std::chrono::high_resolution_clock::now();
				videoSample = this->avReader->PopNextVideoSample();
			}
		}
		else {
			this->lastRenderedVideoSampleTimePoint = std::chrono::high_resolution_clock::now();
			videoSample = this->avReader->PopNextVideoSample();
		}


		if (videoSample) {
			this->lastRenderedVideoSample = std::move(videoSample);
		}


		this->RenderVideoSample(this->lastRenderedVideoSample);

		// Ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		HRESULT hr = d2dCtx->EndDraw();
		if (hr != D2DERR_RECREATE_TARGET) {
			H::System::ThrowIfFailed(hr);
		}

		d2dCtx->RestoreDrawingState(this->stateBlock.Get());

		this->avReader->RequestNeccessaryVideoSamples();
	}

	void VideoSceneRenderer::RenderVideoSample(const std::unique_ptr<MF::MFVideoSample>& videoSample) {
		auto dxDev = this->swapChainPanel->GetDxDevice()->Lock();
		auto dxCtx = dxDev->LockContext();
		auto d2dCtx = dxCtx->D2D();

		if (videoSample) {
			auto dpiScaleFactor = this->swapChainPanel->GetDpi() / 96.0f;

			// Create a ID2D1Bitmap1 out of the frame texture
			auto frame = DxTools::CreateFrameBitmap(dxCtx, videoSample->texture);
			this->scaleEffect->SetInput(0, frame.Get());

			D3D11_TEXTURE2D_DESC frameDesc;
			videoSample->texture->GetDesc(&frameDesc);


			// Scale the frame keeping aspect ratio
			auto renderTargetSize = this->swapChainPanel->GetRenderTargetSize();
			float scaleX = static_cast<float>(renderTargetSize.width) / frameDesc.Width;
			float scaleY = static_cast<float>(renderTargetSize.height) / frameDesc.Height;
			float scaleFactor = (std::min)(scaleX, scaleY);
			this->scaleEffect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(scaleFactor, scaleFactor));

			Microsoft::WRL::ComPtr<ID2D1Image> pScaledFrame;
			this->scaleEffect->GetOutput(&pScaledFrame);
			this->centerEffect->SetInput(0, pScaledFrame.Get());

			// NOTE: For scaleEffect keep 'scaleFactor' as it is. But for centerEffect 'scaleX' & 'scaleFactor' 
			//       need divide on dpiScaleFactor because swapChain have transform matrix 2x2:
			//		 | 1 / dpiScaleFactor,  0 |
			//		 | 0,  1 / dpiScaleFactor |
			//
			float freeWidthFrame = (frameDesc.Width) * (scaleX - scaleFactor) / dpiScaleFactor; // rest of scale X
			float freeHeightFrame = (frameDesc.Height) * (scaleY - scaleFactor) / dpiScaleFactor; // rest of scale Y
			float offsetX = freeWidthFrame / 2;
			float offsetY = freeHeightFrame / 2;
			D2D1_MATRIX_3X2_F matrix{
				1, 0,
				0, 1,
				offsetX, offsetY
			};
			this->centerEffect->SetValue(D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX, matrix);

			d2dCtx->DrawImage(this->centerEffect.Get());
		}
	}
}
#endif