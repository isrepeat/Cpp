#pragma once
#include "../Common/Config.h"
#if ENGINE_TYPE == DX_PLAYER_RENDER
#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include "ShaderStructures.h"
#include "AvReader.h"
#include <Helpers/Rational.h>
#include <Helpers/Dx/DxHelpers.h>

#define VIDEO_RENDERER 1

namespace DxPlayer
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();

#if VIDEO_RENDERER
		void Init(std::unique_ptr<AvReader> avReader);
#endif
		void Update(DX::StepTimer const& timer);
		void Render();

		void StartTracking();
		void TrackingUpdate(float positionX);
		void StopTracking();
		bool IsTracking();


	private:
		void Rotate(float radians);
#if VIDEO_RENDERER
		void RenderVideoSample(const std::unique_ptr<MF::MFVideoSample>& videoSample);
#endif

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		
#if VIDEO_RENDERER
		std::unique_ptr<AvReader> avReader;
		const H::Rational<double> monitorRefreshRate;
		const H::Chrono::milliseconds_f monitorRefreshDuration;
		std::unique_ptr<MF::MFVideoSample> lastRenderedVideoSample;
		std::chrono::steady_clock::time_point lastRenderedVideoSampleTimePoint;
#endif

		// Direct2D resources
		Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1> m_stateBlock;
		Microsoft::WRL::ComPtr<ID2D1Effect> m_pScaleEffect;
		Microsoft::WRL::ComPtr<ID2D1Effect> m_pCenterEffect;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;
	};
}
#endif