#pragma once
#include <Helpers/MediaFoundation/MFInclude.h>
#include <Helpers/Time.h>
#include <mmdeviceapi.h>
#include <wrl.h>

namespace MFTools {
	namespace SourceReader {
		HRESULT GetSourceDuration(Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader, H::Chrono::Hns& duration);
		HRESULT GetSourceFlags(Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader, ULONG& flags);
		HRESULT SetPosition(Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader, H::Chrono::Hns position);
		BOOL SourceCanSeek(Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader);
	}

	H::Chrono::Hns GetSampleDuration(IMFSample* pSample);
	H::Chrono::Hns GetSampleTime(IMFSample* pSample);

	// Gets first supported media type of a given stream sink
	HRESULT GetFirstSupportedMediaType(
		Microsoft::WRL::ComPtr<IMFStreamSink> pStreamSink,
		Microsoft::WRL::ComPtr<IMFMediaType>& pType
	);
}