#include "MFTools.h"
#include <Helpers/System.h>
#include <propvarutil.h>
#include "MFTools.h"

#pragma comment(lib, "propsys.lib")

namespace MFTools {
	namespace SourceReader {
		HRESULT GetSourceDuration(Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader, H::Chrono::Hns& duration) {
			if (!sourceReader)
				return E_POINTER;

			PROPVARIANT var;
			PropVariantInit(&var);

			HRESULT hr = sourceReader->GetPresentationAttribute(
				MF_SOURCE_READER_MEDIASOURCE,
				MF_PD_DURATION,
				&var
			);

			if (SUCCEEDED(hr)) {
				duration = H::Chrono::Hns(var.hVal.QuadPart);
			}

			PropVariantClear(&var);
			return hr;
		}

		HRESULT GetSourceFlags(Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader, ULONG& flags) {
			if (!sourceReader)
				return E_POINTER;

			PROPVARIANT var;
			PropVariantInit(&var);

			HRESULT hr = sourceReader->GetPresentationAttribute(
				MF_SOURCE_READER_MEDIASOURCE,
				MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS,
				&var);

			if (SUCCEEDED(hr)) {
				flags = var.hVal.QuadPart;
			}

			PropVariantClear(&var);
			return hr;
		}

		HRESULT SetPosition(Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader, H::Chrono::Hns position) {
			if (!sourceReader)
				return E_POINTER;

			HRESULT hr = S_OK;

			PROPVARIANT pv;
			pv.vt = VT_I8;
			pv.hVal.QuadPart = (int64_t)position;
			hr = sourceReader->SetCurrentPosition(GUID_NULL, pv);
			return hr;
		}

		BOOL SourceCanSeek(Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader) {
			if (!sourceReader)
				return FALSE;

			ULONG flags;
			HRESULT hr = GetSourceFlags(sourceReader, flags);
			if (FAILED(hr))
				return FALSE;

			BOOL bCanSeek = ((flags & MFMEDIASOURCE_CAN_SEEK) == MFMEDIASOURCE_CAN_SEEK);
			return bCanSeek;
		}
	} // namespace SourceReader


	H::Chrono::Hns GetSampleDuration(IMFSample* pSample) {
		if (!pSample) {
			assert(false);
			return {};
		}
		int64_t sampleDuration;
		pSample->GetSampleDuration(&sampleDuration);
		return H::Chrono::Hns{ sampleDuration };
	}

	H::Chrono::Hns GetSampleTime(IMFSample* pSample) {
		if (!pSample) {
			assert(false);
			return {};
		}
		int64_t sampleTime;
		pSample->GetSampleTime(&sampleTime);
		return H::Chrono::Hns{ sampleTime };
	}


	HRESULT GetFirstSupportedMediaType(Microsoft::WRL::ComPtr<IMFStreamSink> pStreamSink, Microsoft::WRL::ComPtr<IMFMediaType>& pType) {
		HRESULT hr = S_OK;

		Microsoft::WRL::ComPtr<IMFMediaTypeHandler> pTypeHandler;
		hr = pStreamSink->GetMediaTypeHandler(&pTypeHandler);
		H::System::ThrowIfFailed(hr);

		DWORD sinkMediaTypeCount = 0;
		hr = pTypeHandler->GetMediaTypeCount(&sinkMediaTypeCount);
		H::System::ThrowIfFailed(hr);

		for (UINT i = 0; i < sinkMediaTypeCount; i++) {
			hr = pTypeHandler->GetMediaTypeByIndex(i, &pType);
			H::System::ThrowIfFailed(hr);

			if (pTypeHandler->IsMediaTypeSupported(pType.Get(), NULL) == S_OK)
				break;
		}

		if (!pType)
			return HRESULT_FROM_WIN32(ERROR_UNSUPPORTED_TYPE);

		hr = pTypeHandler->SetCurrentMediaType(pType.Get());
		H::System::ThrowIfFailed(hr);

		return hr;
	}
} // namespace MFTools