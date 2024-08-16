#pragma once
#include <Helpers/MediaFoundation/SampleInfo.h>
//#include <Helpers/Dx/DxDevice.h>
#include <Helpers/Dx/SwapChainPanel.h>
#include <Helpers/ConcurrentQueue.h>
#include <Helpers/Rational.h>

#include <MediaEngine/AvReader.h>
#include <mutex>

#define SIMPLE_RENDER 1

namespace MediaEngineWinRT {
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	class VideoSceneRenderer {
	public:
		VideoSceneRenderer(Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel);
		~VideoSceneRenderer();

		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();


		void Init(std::unique_ptr<AvReader> avReader);
#if !ASYNC_READER
		void ReadNeccessaryVideoSamples();
#endif
		void Render();

	private:
#if !ASYNC_READER
		void RequestNextVideoSample();
#endif
		void RenderVideoSample(const std::unique_ptr<MF::MFVideoSample>& videoSample);

	private:
		std::mutex mx;
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel;

		std::unique_ptr<AvReader> avReader;
		const H::Rational<double> monitorRefreshRate;
		const H::Chrono::milliseconds_f monitorRefreshDuration;
		std::unique_ptr<MF::MFVideoSample> nextVideoSample;
		std::unique_ptr<MF::MFVideoSample> lastRenderedVideoSample;
		std::chrono::steady_clock::time_point lastRenderedVideoSampleTimePoint;

		// Direct2D resources
		Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1> stateBlock;
		Microsoft::WRL::ComPtr<ID2D1Effect> scaleEffect;
		Microsoft::WRL::ComPtr<ID2D1Effect> centerEffect;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		//Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
		//Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer constantBufferData;

		// Variables used with the rendering loop.
		bool loadingComplete;
		bool queryNextSample = false;
	};
}