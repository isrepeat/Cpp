#include "pch.h"
#if ENGINE_TYPE == DX_PLAYER_RENDER
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

#pragma comment(lib, "dxguid.lib")

using namespace DirectX;
using namespace Windows::Foundation;


namespace DxTools {
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> CreateFrameBitmap(
		DX::DeviceResources::_Locked& dxDevResources,
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
		hr = dxDevResources->GetD2DDeviceContext()->CreateBitmapFromDxgiSurface(
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
	// Loads vertex and pixel shaders from files and instantiates the cube geometry.
	Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources)
		: m_loadingComplete(false)
		, m_degreesPerSecond(45)
		, m_indexCount(0)
		, m_tracking(false)
		, m_deviceResources(deviceResources)
#if VIDEO_RENDERER
		, monitorRefreshRate{ H::Dx::GetRefreshRateForDXGIOutput(H::Dx::EnumOutputsState{ H::Dx::EnumAdaptersState{}.Next() }.Next().GetDXGIOutput()) }
		, monitorRefreshDuration{ monitorRefreshRate.Inversed().As<H::Chrono::seconds_f>() }
#endif
	{
		DX::ThrowIfFailed(
			m_deviceResources->Lock()->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
		);

		CreateDeviceDependentResources();
		CreateWindowSizeDependentResources();
	}



	void Sample3DSceneRenderer::CreateDeviceDependentResources() {
		// Load shaders asynchronously.
		auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
		auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

		// After the vertex shader file is loaded, create the shader and input layout.
		auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
			auto dxDevResources = m_deviceResources->Lock();
			DX::ThrowIfFailed(
				dxDevResources->GetD3DDevice()->CreateVertexShader(
					&fileData[0],
					fileData.size(),
					nullptr,
					&m_vertexShader
				)
			);

			static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			DX::ThrowIfFailed(
				dxDevResources->GetD3DDevice()->CreateInputLayout(
					vertexDesc,
					ARRAYSIZE(vertexDesc),
					&fileData[0],
					fileData.size(),
					&m_inputLayout
				)
			);
			});

		// After the pixel shader file is loaded, create the shader and constant buffer.
		auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
			auto dxDevResources = m_deviceResources->Lock();
			DX::ThrowIfFailed(
				dxDevResources->GetD3DDevice()->CreatePixelShader(
					&fileData[0],
					fileData.size(),
					nullptr,
					&m_pixelShader
				)
			);

			CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
			DX::ThrowIfFailed(
				dxDevResources->GetD3DDevice()->CreateBuffer(
					&constantBufferDesc,
					nullptr,
					&m_constantBuffer
				)
			);
			});

		// Once both shaders are loaded, create the mesh.
		auto createCubeTask = (createPSTask && createVSTask).then([this] {
			auto dxDevResources = m_deviceResources->Lock();

			// Load mesh vertices. Each vertex has a position and a color.
			static const VertexPositionColor cubeVertices[] = {
				{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
				{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
				{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
				{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
				{XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
				{XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
				{XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
				{XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
			};

			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = cubeVertices;
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				dxDevResources->GetD3DDevice()->CreateBuffer(
					&vertexBufferDesc,
					&vertexBufferData,
					&m_vertexBuffer
				)
			);

			// Load mesh indices. Each trio of indices represents
			// a triangle to be rendered on the screen.
			// For example: 0,2,1 means that the vertices with indexes
			// 0, 2 and 1 from the vertex buffer compose the 
			// first triangle of this mesh.
			static const unsigned short cubeIndices[] = {
				0,2,1, // -x
				1,2,3,

				4,5,6, // +x
				5,7,6,

				0,1,5, // -y
				0,5,4,

				2,6,7, // +y
				2,7,3,

				0,4,6, // -z
				0,6,2,

				1,3,7, // +z
				1,7,5,
			};

			m_indexCount = ARRAYSIZE(cubeIndices);

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = cubeIndices;
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				dxDevResources->GetD3DDevice()->CreateBuffer(
					&indexBufferDesc,
					&indexBufferData,
					&m_indexBuffer
				)
			);
			});

		// Once the cube is loaded, the object is ready to be rendered.
		createCubeTask.then([this]() {
			m_loadingComplete = true;
			});


		HRESULT hr = S_OK;
		auto dxDevResources = m_deviceResources->Lock();

		// Create frame transformation effects
		hr = dxDevResources->GetD2DDeviceContext()->CreateEffect(CLSID_D2D1Scale, &m_pScaleEffect);
		H::System::ThrowIfFailed(hr);

		hr = dxDevResources->GetD2DDeviceContext()->CreateEffect(CLSID_D2D12DAffineTransform, &m_pCenterEffect);
		H::System::ThrowIfFailed(hr);
	}
	
	// Initializes view parameters when the window size changes.
	void Sample3DSceneRenderer::CreateWindowSizeDependentResources() {
		auto dxDevResources = m_deviceResources->Lock();
		Size outputSize = dxDevResources->GetOutputSize();
		float aspectRatio = outputSize.Width / outputSize.Height;
		float fovAngleY = 70.0f * XM_PI / 180.0f;

		// This is a simple example of change that can be made when the app is in
		// portrait or snapped view.
		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		// Note that the OrientationTransform3D matrix is post-multiplied here
		// in order to correctly orient the scene to match the display orientation.
		// This post-multiplication step is required for any draw calls that are
		// made to the swap chain render target. For draw calls to other targets,
		// this transform should not be applied.

		// This sample makes use of a right-handed coordinate system using row-major matrices.
		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
			fovAngleY,
			aspectRatio,
			0.01f,
			100.0f
		);

		XMFLOAT4X4 orientation = dxDevResources->GetOrientationTransform3D();

		XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		XMStoreFloat4x4(
			&m_constantBufferData.projection,
			XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

		// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
		static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
		static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
		static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
	}

	void Sample3DSceneRenderer::ReleaseDeviceDependentResources() {
		m_loadingComplete = false;
		m_vertexShader.Reset();
		m_inputLayout.Reset();
		m_pixelShader.Reset();
		m_constantBuffer.Reset();
		m_vertexBuffer.Reset();
		m_indexBuffer.Reset();
	}

#if VIDEO_RENDERER
	void Sample3DSceneRenderer::Init(std::unique_ptr<AvReader> avReader) {
		this->avReader = std::move(avReader);
	}
#endif


	// Called once per frame, rotates the cube and calculates the model and view matrices.
	void Sample3DSceneRenderer::Update(DX::StepTimer const& timer) {
#if !VIDEO_RENDERER
		if (!m_tracking)
		{
			// Convert degrees to radians, then convert seconds to rotation angle
			float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
			double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
			float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

			Rotate(radians);
		}
#endif
	}


	// Renders one frame using the vertex and pixel shaders.
	void Sample3DSceneRenderer::Render() {
		// Loading is asynchronous. Only draw geometry after it's loaded.
		if (!m_loadingComplete) {
			return;
		}

#if VIDEO_RENDERER
		auto dxDevResources = m_deviceResources->Lock();
		ID2D1DeviceContext* d2dCtx = dxDevResources->GetD2DDeviceContext();
		Windows::Foundation::Size logicalSize = dxDevResources->GetLogicalSize();

		d2dCtx->SaveDrawingState(m_stateBlock.Get());
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
			DX::ThrowIfFailed(hr);
		}

		d2dCtx->RestoreDrawingState(m_stateBlock.Get());


		this->avReader->RequestNeccessaryVideoSamples();
#else
		auto dxDevResources = m_deviceResources->Lock();
		auto d3dCtx = dxDevResources->GetD3DDeviceContext();

		// Prepare the constant buffer to send it to the graphics device.
		d3dCtx->UpdateSubresource1(
			m_constantBuffer.Get(),
			0,
			NULL,
			&m_constantBufferData,
			0,
			0,
			0
		);

		// Each vertex is one instance of the VertexPositionColor struct.
		UINT stride = sizeof(VertexPositionColor);
		UINT offset = 0;
		d3dCtx->IASetVertexBuffers(
			0,
			1,
			m_vertexBuffer.GetAddressOf(),
			&stride,
			&offset
		);

		d3dCtx->IASetIndexBuffer(
			m_indexBuffer.Get(),
			DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
			0
		);

		d3dCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		d3dCtx->IASetInputLayout(m_inputLayout.Get());

		// Attach our vertex shader.
		d3dCtx->VSSetShader(
			m_vertexShader.Get(),
			nullptr,
			0
		);

		// Send the constant buffer to the graphics device.
		d3dCtx->VSSetConstantBuffers1(
			0,
			1,
			m_constantBuffer.GetAddressOf(),
			nullptr,
			nullptr
		);

		// Attach our pixel shader.
		d3dCtx->PSSetShader(
			m_pixelShader.Get(),
			nullptr,
			0
		);

		// Draw the objects.
		d3dCtx->DrawIndexed(
			m_indexCount,
			0,
			0
		);
#endif
	}


	void Sample3DSceneRenderer::StartTracking() {
		m_tracking = true;
	}

	// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
	void Sample3DSceneRenderer::TrackingUpdate(float positionX) {
		if (m_tracking) {
			auto dxDevResources = m_deviceResources->Lock();
			float radians = XM_2PI * 2.0f * positionX / dxDevResources->GetOutputSize().Width;
			Rotate(radians);
		}
	}

	void Sample3DSceneRenderer::StopTracking() {
		m_tracking = false;
	}

	bool Sample3DSceneRenderer::IsTracking() {
		return m_tracking;
	}



	// Rotate the 3D cube model a set amount of radians.
	void Sample3DSceneRenderer::Rotate(float radians) {
		// Prepare to pass the updated model matrix to the shader
		XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
	}

#if VIDEO_RENDERER
	void Sample3DSceneRenderer::RenderVideoSample(const std::unique_ptr<MF::MFVideoSample>& videoSample) {
		auto dxDevResources = m_deviceResources->Lock();
		auto d2dCtx = dxDevResources->GetD2DDeviceContext();

		if (videoSample) {
			auto dpiScaleFactor = dxDevResources->GetDpi() / 96.0f;

			// Create a ID2D1Bitmap1 out of the frame texture
			auto frame = DxTools::CreateFrameBitmap(dxDevResources, videoSample->texture);
			m_pScaleEffect->SetInput(0, frame.Get());

			D3D11_TEXTURE2D_DESC frameDesc;
			videoSample->texture->GetDesc(&frameDesc);


			// Scale the frame keeping aspect ratio
			auto renderTargetSize = dxDevResources->GetRenderTargetSize();
			float scaleX = static_cast<float>(renderTargetSize.Width) / frameDesc.Width;
			float scaleY = static_cast<float>(renderTargetSize.Height) / frameDesc.Height;
			float scaleFactor = (std::min)(scaleX, scaleY);
			m_pScaleEffect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(scaleFactor, scaleFactor));

			Microsoft::WRL::ComPtr<ID2D1Image> pScaledFrame;
			m_pScaleEffect->GetOutput(&pScaledFrame);
			m_pCenterEffect->SetInput(0, pScaledFrame.Get());

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
			m_pCenterEffect->SetValue(D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX, matrix);

			d2dCtx->DrawImage(m_pCenterEffect.Get());
		}
	}
#endif
}
#endif