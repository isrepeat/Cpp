#include "AvReader.h"
#include "MFTools.h"

#define LOG_DEBUG_D(fmt, ...)

AvReader::AvReader(IStream* stream, H::Dx::DxDeviceSafeObj* dxDeviceSafeObj)
	: readerCallback{ Microsoft::WRL::Make<ReaderCallback>(this) }
	, dxDeviceSafeObj{ dxDeviceSafeObj }
	, avReaderDxgiManager{ std::make_unique<AvReaderDxgiManager>(dxDeviceSafeObj) }
	//, requestsSamplesQueue{ std::make_shared<H::ConcurrentQueue<H::movable_function<void()>>>() }
	, requestsSamplesQueue{ std::make_shared<H::ConcurrentQueue<std::function<void()>>>() }
{
	this->InitReader(stream);
}

AvReader::~AvReader() {
	this->requestsSamplesQueue->StopWork();

	if (this->requestsSamplesRoutine.valid())
		this->requestsSamplesRoutine.wait();
}


H::Chrono::Hns AvReader::GetSourceDuration() {
	return this->sourceDuration;
}

AvReader::_AvSourceStreamManagerSafeObj::_Locked AvReader::GetLockedAvSourceStreamManager() {
	return avSourceStreamManagerSafeObj->Lock();
}

void AvReader::SeekAsync(AvReaderRewindParams rewindParams) {
	std::unique_lock lk{ mx };
	if (auto rewindAsyncResult = this->rewindAsyncResultWeak.lock()) {
		if (rewindAsyncResult->GetStatus() == AvReaderRewindAsyncResult::Status::Pending) {
			LOG_DEBUG_D("rewindAsyncResult not completed, ignore");
			return;
		}
	}

	this->lastSeekPosition = rewindParams.pts;

	HRESULT hr = this->sourceReader->Flush(MF_SOURCE_READER_ALL_STREAMS);
	if (SUCCEEDED(hr)) {
		this->rewindAsyncResultWeak = rewindParams.asyncResultWeak;
		if (auto rewindAsyncResult = this->rewindAsyncResultWeak.lock()) {
			rewindAsyncResult->SeekStart(avSourceStreamManagerSafeObj->Lock());
		}
	}
	else {
		LOG_FAILED(hr);
		Dbreak;
	}
}


std::unique_ptr<MF::MFVideoSample> AvReader::ReadNextVideoSample() {
	//LOG_DEBUG_D("ReadNextVideoSample()");
	HRESULT hr = S_OK;
	_In_ DWORD dwActualStreamIndex = -1;
	_In_ DWORD dwStreamFlags = -1;
	_In_ LONGLONG llTimestamp = -1;
	_In_opt_ Microsoft::WRL::ComPtr<IMFSample> pSample;

	std::unique_ptr<MF::MFVideoSample> mfVideoSample;

	auto avSourceStreamManager = avSourceStreamManagerSafeObj->Lock();
	if (auto videoStream = avSourceStreamManager->GetActiveVideoStream()) {
		//LOG_DEBUG_D("Request Video sample");
		hr = this->sourceReader->ReadSample(videoStream->GetStreamIdx(), 0, &dwActualStreamIndex, &dwStreamFlags, &llTimestamp, &pSample);
		if (SUCCEEDED(hr)) {
		//	LOG_DEBUG_D("pSample = {}", static_cast<void*>(pSample));
		//	LOG_DEBUG_D("GotSample(streamIndex = {}, currPos = {})"
		//		, dwActualStreamIndex
		//		, H::Chrono::Hns{ llTimestamp }
		//	);

			auto mfSample = std::make_unique<MF::MFSample>(
				MFTools::GetSampleTime(pSample.Get()),
				MFTools::GetSampleDuration(pSample.Get()),
				pSample);

			mfVideoSample = videoStream->GetEffect()->Process(std::move(mfSample));
		}
	}

	//auto mfVideoSample = std::make_unique<MF::MFVideoSample>(MF::MFSample{
	//		MFTools::GetSampleTime(pSample.Get()),
	//		MFTools::GetSampleDuration(pSample.Get()),
	//		pSample
	//	});

	return mfVideoSample;
}


#if ASYNC_READER
void AvReader::RequestNeccessaryVideoSamples() {
	std::unique_lock lk{ mx };
	if (auto rewindAsyncResult = this->rewindAsyncResultWeak.lock()) {
		if (rewindAsyncResult->GetStatus() == AvReaderRewindAsyncResult::Status::Pending) {
			LOG_DEBUG_D("rewindAsyncResult not completed, ignore");
			return;
		}
	}

	//requestsSamplesQueue->Push({ "RequestNeccessaryVideoSamples::lambda__", [this] {
		HRESULT hr = S_OK;
		auto avSourceStreamManager = avSourceStreamManagerSafeObj->Lock();
		auto activeVideoStream = avSourceStreamManager->GetActiveVideoStream();
		if (!activeVideoStream) {
			LOG_WARNING_D("Stream is not active");
			return;
		}

		if (activeVideoStream->GetSamplesRequests() == 0) {
			this->RequestNextSampleInternal(avSourceStreamManager, AvStreamType::Video);
		}
		//} });
}
#else
void AvReader::RequestNeccessaryVideoSamples() {
	//std::unique_lock lk{ mx };
	//if (auto rewindAsyncResult = this->rewindAsyncResultWeak.lock()) {
	//	if (rewindAsyncResult->GetStatus() == AvReaderRewindAsyncResult::Status::Pending) {
	//		LOG_DEBUG_D("rewindAsyncResult not completed, ignore");
	//		return;
	//	}
	//}
	int xxx = 9;
	requestsSamplesQueue->Push([this, xxx] {
		auto tp1 = std::chrono::high_resolution_clock::now();
		//std::unique_lock lk{ mx };
		
		//auto dxDev = this->dxDeviceSafeObj->Lock();
		//auto dxCtx = dxDev->LockContext();

		HRESULT hr = S_OK;
		auto avSourceStreamManager = avSourceStreamManagerSafeObj->Lock();
		auto activeVideoStream = avSourceStreamManager->GetActiveVideoStream();
		if (!activeVideoStream) {
			LOG_WARNING_D("Stream is not active");
			return;
		}

		auto tp2 = std::chrono::high_resolution_clock::now();

		if (activeVideoStream->GetSamplesRequests() == 0) {
			this->RequestNextSampleInternal(avSourceStreamManager, AvStreamType::Video);
		}

		auto tp3 = std::chrono::high_resolution_clock::now();
		//LOG_DEBUG_D("\n"
		//	"dt [Lock] = {}\n"
		//	"dt [Request sample] = {}\n"
		//	, tp2 - tp1
		//	, tp3 - tp2
		//);
		});
}
#endif

void AvReader::RequestNeccessaryAudioSamples() {
	std::unique_lock lk{ mx };
	//if (auto rewindAsyncResult = this->rewindAsyncResultWeak.lock()) {
	//	if (rewindAsyncResult->GetStatus() == AvReaderRewindAsyncResult::Status::Pending) {
	//		LOG_DEBUG_D("rewindAsyncResult not completed, ignore");
	//		return;
	//	}
	//}

	//requestsSamplesQueue->Push({ "RequestNeccessaryAudioSamples::lambda__", [this] {
		HRESULT hr = S_OK;
		auto avSourceStreamManager = avSourceStreamManagerSafeObj->Lock();
		auto activeAudioStream = avSourceStreamManager->GetActiveAudioStream();
		if (!activeAudioStream) {
			LOG_WARNING_D("Stream is not active");
			return;
		}

		if (activeAudioStream->GetSamplesRequests() < 5) {
			this->RequestNextSampleInternal(avSourceStreamManager, AvStreamType::Audio);
		}
		//} });
}


std::unique_ptr<MF::MFVideoSample> AvReader::PopNextVideoSample() {
	auto avSourceStreamManager = avSourceStreamManagerSafeObj->Lock();
	auto activeVideoStream = avSourceStreamManager->GetActiveVideoStream();
	//if (LOG_ASSERT(activeVideoStream, "Stream is not active")) {
	if (!activeVideoStream) {
		LOG_WARNING_D("Stream is not active");
		return nullptr;
	}

	std::unique_ptr<MF::MFVideoSample> videoSample = H::dynamic_unique_cast<MF::MFVideoSample>(activeVideoStream->Pop());
	return videoSample;
}


std::unique_ptr<MF::MFAudioSample> AvReader::PopNextAudioSample() {
	auto avSourceStreamManager = avSourceStreamManagerSafeObj->Lock();
	auto activeAudioStream = avSourceStreamManager->GetActiveAudioStream();
	if (LOG_ASSERT(activeAudioStream, "Stream is not active")) {
		return nullptr;
	}

	std::unique_ptr<MF::MFAudioSample> audioSample = H::dynamic_unique_cast<MF::MFAudioSample>(activeAudioStream->Pop());
	return audioSample;
}




void AvReader::InitReader(IStream* stream) {
	HRESULT hr = S_OK;

	// Configure the source reader
	Microsoft::WRL::ComPtr<IMFAttributes> pAttributes;
	hr = MFCreateAttributes(&pAttributes, 4);
	H::System::ThrowIfFailed(hr);

#if ASYNC_READER
	hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this->readerCallback.Get());
	H::System::ThrowIfFailed(hr);
#endif

	hr = pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
	H::System::ThrowIfFailed(hr);

	if (this->avReaderDxgiManager) {
		this->avReaderDxgiManager->SetAttributes(pAttributes.Get());
	}

	// Create the media source reader
	Microsoft::WRL::ComPtr<IMFByteStream> pByteStream;
	hr = MFCreateMFByteStreamOnStreamEx(stream, &pByteStream);
	H::System::ThrowIfFailed(hr);

	hr = MFCreateSourceReaderFromByteStream(pByteStream.Get(), pAttributes.Get(), this->sourceReader.ReleaseAndGetAddressOf());
	H::System::ThrowIfFailed(hr);

	hr = MFTools::SourceReader::GetSourceDuration(this->sourceReader, this->sourceDuration);
	H::System::ThrowIfFailed(hr);

	this->avSourceStreamManagerSafeObj = std::make_unique<_AvSourceStreamManagerSafeObj>(this->sourceReader);
	
	this->ConfigureDecoders();
}


HRESULT AvReader::ConfigureDecoders() {
	HRESULT hr = S_OK;

	auto avSourceStreamManager = this->avSourceStreamManagerSafeObj->Lock();

	// Determine real video/audio stream idx
	this->videoStreamIdx = MF_SOURCE_READER_INVALID_STREAM_INDEX;
	this->audioStreamIdx = MF_SOURCE_READER_INVALID_STREAM_INDEX;

	auto streamCount = GetStreamCountFromReaderInternal();

	for (int streamIdx = 0; streamIdx < streamCount; streamIdx++) {
		BOOL isStreamSelected;
		hr = sourceReader->GetStreamSelection(streamIdx, &isStreamSelected);
		H::System::ThrowIfFailed(hr);

		if (!isStreamSelected)
			continue;

		Microsoft::WRL::ComPtr<IMFMediaType> currentMediaType;
		hr = sourceReader->GetCurrentMediaType(streamIdx, currentMediaType.ReleaseAndGetAddressOf());
		if (FAILED(hr)) {
			continue;
		}

		GUID majorType;
		hr = currentMediaType->GetMajorType(&majorType);
		if (FAILED(hr)) {
			continue;
		}

		if (majorType == MFMediaType_Video) {
			// Set decoded video sample format for first found video stream
			if (this->videoStreamIdx == MF_SOURCE_READER_INVALID_STREAM_INDEX) {
				this->videoStreamIdx = streamIdx;

				Microsoft::WRL::ComPtr<IMFMediaType> decodedVideoType;
				MFCreateMediaType(&decodedVideoType);

				hr = currentMediaType->CopyAllItems(decodedVideoType.Get());
				H::System::ThrowIfFailed(hr);

				hr = decodedVideoType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
				H::System::ThrowIfFailed(hr);

				//MF::Helpers::PrintAudioMediaTypeInfo(decodedVideoType, "[decodedVideoType]");

				hr = this->sourceReader->SetCurrentMediaType(this->firstVideoStreamIdx, NULL, decodedVideoType.Get());
				H::System::ThrowIfFailed(hr);

				auto avReaderDxgiEffect = H::dynamic_unique_cast<AvReaderDxgiEffect>(this->avReaderDxgiManager->CreateEffect(decodedVideoType.Get()));
				
				std::unique_ptr<AvStreamVideo> videoStream = std::make_unique<AvStreamVideo>(
					streamIdx,
					H::Size{ 0, 0 },
					TextureType::ARGB32,
					H::Rational<float>{0, 1},
					0,
					0,
					0,
					std::move(avReaderDxgiEffect)
					);

				avSourceStreamManager->AddStream(std::move(videoStream));
			}
		}
		else if (majorType == MFMediaType_Audio) {
			//// Set decoded audio sample format for first found audio stream
			//if (this->audioStreamIdx == MF_SOURCE_READER_INVALID_STREAM_INDEX) {
			//	this->audioStreamIdx = streamIdx;

			//	Microsoft::WRL::ComPtr<IMFMediaType> decodedAudioType;
			//	MFCreateMediaType(&decodedAudioType);

			//	hr = currentMediaType->CopyAllItems(decodedAudioType.Get());
			//	H::System::ThrowIfFailed(hr);

			//	//hr = decodedAudioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
			//	hr = decodedAudioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
			//	H::System::ThrowIfFailed(hr);

			//	//MF::Helpers::PrintAudioMediaTypeInfo(decodedAudioType, "[decodedAudioType]");

			//	hr = this->sourceReader->SetCurrentMediaType(this->firstAudioStreamIdx, NULL, decodedAudioType.Get());
			//	H::System::ThrowIfFailed(hr);

			//	std::unique_ptr<AvStreamAudio> stream = std::make_unique<AvStreamAudio>(
			//		streamIdx,
			//		0, //waveFmt.nChannels,
			//		0, //waveFmt.nSamplesPerSec,
			//		0, //avgBytesPerSecond,
			//		nullptr
			//		);
			//}
		}
	}

	avSourceStreamManager->DisableAllStreams();
	avSourceStreamManager->SetActiveStream(avSourceStreamManager->GetFirstVideoStream());
	//avSourceStreamManager->SetActiveStream(avSourceStreamManager->GetFirstAudioStream());


	auto requestsSamplesQueueCopy = requestsSamplesQueue;
	this->requestsSamplesRoutine = std::async(std::launch::async, [requestsSamplesQueueCopy, this] {
		LOG_THREAD(L"requestsSamples thread");
		while (requestsSamplesQueueCopy->IsWorking()) {
			auto task = requestsSamplesQueueCopy->Pop();
			if (task) {
				task();
			}
		}
		});

	return hr;
}


DWORD AvReader::GetStreamCountFromReaderInternal() {
	HRESULT hr = S_OK;
	DWORD streamCount = 0;
	Microsoft::WRL::ComPtr<IMFMediaSource> source;
	Microsoft::WRL::ComPtr<IMFPresentationDescriptor> presentationDesc;

	hr = sourceReader->GetServiceForStream(MF_SOURCE_READER_MEDIASOURCE, GUID_NULL, IID_PPV_ARGS(source.GetAddressOf()));
	H::System::ThrowIfFailed(hr);

	hr = source->CreatePresentationDescriptor(presentationDesc.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	hr = presentationDesc->GetStreamDescriptorCount(&streamCount);
	H::System::ThrowIfFailed(hr);

	return streamCount;
}


#if ASYNC_READER
HRESULT AvReader::RequestNextSampleInternal(_AvSourceStreamManagerSafeObj::_Locked& avSourceStreamManager, AvStreamType streamType) {
	HRESULT hr = S_OK;

	switch (streamType) {
	case AvStreamType::Video:
		if (auto videoStream = avSourceStreamManager->GetActiveVideoStream()) {
			//LOG_DEBUG_D("Request Video sample");
			hr = this->sourceReader->ReadSample(videoStream->GetStreamIdx(), 0, NULL, NULL, NULL, NULL);
			LOG_FAILED(hr);
			if (SUCCEEDED(hr)) {
				videoStream->AddPushRequest();
			}
		}
		break;

	case AvStreamType::Audio:
		if (auto audioStream = avSourceStreamManager->GetActiveAudioStream()) {
			//LOG_DEBUG_D("Request Audio sample");
			hr = this->sourceReader->ReadSample(audioStream->GetStreamIdx(), 0, NULL, NULL, NULL, NULL);
			LOG_FAILED(hr);
			if (SUCCEEDED(hr)) {
				audioStream->AddPushRequest();
			}
		}
		break;

	default:
		hr = E_NOTIMPL;
	}

	return hr;
}
#else
HRESULT AvReader::RequestNextSampleInternal(_AvSourceStreamManagerSafeObj::_Locked& avSourceStreamManager, AvStreamType streamType) {
	HRESULT hr = S_OK;

	_In_ DWORD dwActualStreamIndex = -1;
	_In_ DWORD dwStreamFlags = -1;
	_In_ LONGLONG llTimestamp = -1;
	//_In_opt_ IMFSample* pSample = nullptr;
	_In_opt_ Microsoft::WRL::ComPtr<IMFSample> pSample;

	switch (streamType) {
	case AvStreamType::Video:
		if (auto videoStream = avSourceStreamManager->GetActiveVideoStream()) {
			//LOG_DEBUG_D("Request Video sample");
			hr = this->sourceReader->ReadSample(videoStream->GetStreamIdx(), 0, &dwActualStreamIndex, &dwStreamFlags, &llTimestamp, &pSample);
			if (SUCCEEDED(hr)) {
				videoStream->AddPushRequest();
			}
			else {
				LOG_FAILED(hr);
				return hr;
			}
		}
		break;

	case AvStreamType::Audio:
		if (auto audioStream = avSourceStreamManager->GetActiveAudioStream()) {
			//LOG_DEBUG_D("Request Audio sample");
			hr = this->sourceReader->ReadSample(audioStream->GetStreamIdx(), 0, &dwActualStreamIndex, &dwStreamFlags, &llTimestamp, &pSample);
			if (SUCCEEDED(hr)) {
				audioStream->AddPushRequest();
			}
			else {
				LOG_FAILED(hr);
				return hr;
			}
		}
		break;

	default:
		return E_FAIL;
	}

	return this->OnReadSampleAsync(hr, dwActualStreamIndex, dwStreamFlags, llTimestamp, pSample.Get());
}
#endif


HRESULT AvReader::ProcessVideoSample(std::unique_ptr<MF::MFSample> mfVideoSample) {
	HRESULT hr = S_OK;

	auto avSourceStreamManager = avSourceStreamManagerSafeObj->Lock();
	if (auto activeVideoStream = avSourceStreamManager->GetActiveVideoStream()) {
		activeVideoStream->Push(std::make_unique<MF::MFVideoSample>(*mfVideoSample));
		activeVideoStream->Process(activeVideoStream->GetEffect());
	}


	//// TODO: rewrite to avoid deadlocks and save guard for AvSourceStreamManager.
	//AvStreamVideo* activeVideoStream = nullptr;
	//{
	//	auto avSourceStreamManager = avSourceStreamManagerSafeObj->Lock();
	//	activeVideoStream = avSourceStreamManager->GetActiveVideoStream();
	//	assert(activeVideoStream);

	//	if (auto rewindAsyncResult = this->rewindAsyncResultWeak.lock()) {
	//		if (rewindAsyncResult->GetStatus() == AvReaderRewindAsyncResult::Status::Pending) {
	//			if (this->lastSeekPosition - mfVideoSample->pts > mfVideoSample->duration * 2) {
	//				// Reduce requestsSamples counter
	//				activeVideoStream->Push(nullptr);
	//				activeVideoStream->Process(nullptr);
	//				activeVideoStream->Pop();
	//				return this->RequestNextSampleInternal(avSourceStreamManager, AvStreamType::Video);
	//			}
	//		}
	//	}
	//}

	//// WARNING: potentially deadlock because this thread need lock manager first and dxDevice second,
	////          but render thread lock dxDevice first and can lock manager after ...
	//activeVideoStream->Push(std::make_unique<MF::MFVideoSample>(*mfVideoSample, texture));
	//activeVideoStream->Process(activeVideoStream->GetEffect());

	//if (auto rewindAsyncResult = this->rewindAsyncResultWeak.lock()) {
	//	rewindAsyncResult->NotifyAboutGotFirstVideoSample(static_cast<MF::SampleInfo>(*mfVideoSample));
	//}
	return hr;
}

HRESULT AvReader::ProcessAudioSample(std::unique_ptr<MF::MFSample> mfAudioSample) {
	HRESULT hr = S_OK;

	//BYTE* pAudioBuff;
	//DWORD length;
	//hr = mfAudioSample->buffer->Lock(&pAudioBuff, nullptr, &length);
	//H::System::ThrowIfFailed(hr);

	//std::vector<BYTE> audioData;
	//for (DWORD i = 0; i < length; ++i) {
	//	audioData.push_back(pAudioBuff[i]);
	//}
	//hr = mfAudioSample->buffer->Unlock();
	//H::System::ThrowIfFailed(hr);

	//auto avSourceStreamManager = avSourceStreamManagerSafeObj->Lock();
	//auto activeAudioStream = avSourceStreamManager->GetActiveAudioStream();
	//assert(activeAudioStream);

	//if (auto rewindAsyncResult = this->rewindAsyncResultWeak.lock()) {
	//	if (rewindAsyncResult->GetStatus() == AvReaderRewindAsyncResult::Status::Pending) {
	//		if (this->lastSeekPosition - mfAudioSample->pts > mfAudioSample->duration * 2) {
	//			// Reduce requestsSamples counter
	//			activeAudioStream->Push(nullptr);
	//			activeAudioStream->Process(nullptr);
	//			activeAudioStream->Pop();
	//			return this->RequestNextSampleInternal(avSourceStreamManager, AvStreamType::Audio);
	//		}
	//	}
	//}

	//activeAudioStream->Push(std::make_unique<MFAudioSample>(*mfAudioSample, std::move(audioData)));
	//activeAudioStream->Process(activeAudioStream->GetEffect());

	//if (auto rewindAsyncResult = this->rewindAsyncResultWeak.lock()) {
	//	rewindAsyncResult->NotifyAboutGotFirstAudioSample(static_cast<MF::SampleInfo>(*mfAudioSample));
	//}
	return hr;
}



void AvReader::ClearStream(_AvSourceStreamManagerSafeObj::_Locked& avSourceStreamManager, DWORD dwStreamIndex) {
	if (auto activeVideoStream = avSourceStreamManager->GetActiveVideoStream()) {
		if (dwStreamIndex == activeVideoStream->GetStreamIdx()) {
			LOG_DEBUG_D("Clear video samlples");
			activeVideoStream->ClearSamples(); // mb just clear requested samples conuter?

			if (auto rewindAsyncResult = this->rewindAsyncResultWeak.lock()) {
				rewindAsyncResult->NotifyAboutGotFirstVideoSample({});
			}

			//this->RequestNextSampleInternal(avSourceStreamManager, AvStreamType::Video);
		}
	}

	if (auto activeAudioStream = avSourceStreamManager->GetActiveAudioStream()) {
		if (dwStreamIndex == activeAudioStream->GetStreamIdx()) {
			LOG_DEBUG_D("Clear audio samlples");
			activeAudioStream->ClearSamples();

			if (auto rewindAsyncResult = this->rewindAsyncResultWeak.lock()) {
				rewindAsyncResult->NotifyAboutGotFirstAudioSample({});
			}

			//this->RequestNextSampleInternal(avSourceStreamManager, AvStreamType::Audio);
		}
	}
}

HRESULT AvReader::OnReadSampleAsync(
	_In_ HRESULT hrStatus,
	_In_ DWORD dwStreamIndex,
	_In_ DWORD dwStreamFlags,
	_In_ LONGLONG llTimestamp,
	_In_opt_ IMFSample* pSample)
{
	LOG_DEBUG_D("OnReadSample(streamIdx = {}, currPos = {})"
		, dwStreamIndex
		, H::Chrono::Hns{ llTimestamp }
	);

//#if ASYNC_READER
	// NOTE: mx also must guarantee that avSourceStreamManagerSafeObj will not change any stream indices
	std::unique_lock lk{ mx };
//#endif
	HRESULT hr = S_OK;

	try {		 
		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			LOG_DEBUG_D("END OF STREAM");
			this->ClearStream(this->avSourceStreamManagerSafeObj->Lock(), dwStreamFlags);
		}

		if (!pSample) {
			LOG_WARNING_D("SAMPLE IS NULL");
			this->ClearStream(this->avSourceStreamManagerSafeObj->Lock(), dwStreamFlags);
			return S_OK;
		}

		// Determine sample type
		Microsoft::WRL::ComPtr<IMFMediaType> pType;
		GUID mediaType;
		hr = this->sourceReader->GetCurrentMediaType(dwStreamIndex, &pType);
		H::System::ThrowIfFailed(hr);

		hr = pType->GetMajorType(&mediaType);
		H::System::ThrowIfFailed(hr);

		auto mfSample = std::make_unique<MF::MFSample>(
			MFTools::GetSampleTime(pSample),
			MFTools::GetSampleDuration(pSample),
			pSample
			);

		// Render received media sample
		if (mediaType == MFMediaType_Video) {
			hr = ProcessVideoSample(std::move(mfSample));
		}
		else if (mediaType == MFMediaType_Audio) {
			hr = ProcessAudioSample(std::move(mfSample));
		}

		return S_OK;
	}
	catch (...) {
		LOG_ERROR_D("catch exception");
		Dbreak;

		this->ClearStream(this->avSourceStreamManagerSafeObj->Lock(), dwStreamFlags);
		hr = E_FAIL;
	}
	return hr;
}

HRESULT AvReader::OnFlushAsync(DWORD dwStreamIndex) {
	std::unique_lock lk{ mx };

	HRESULT hr = S_OK;
	auto avSourceStreamManager = avSourceStreamManagerSafeObj->Lock();

	// NOTE: lastSeekPosition can be changed multiple times, and here will be used the last value
	LOG_DEBUG_D("Set reader pos to lastSeekPosition = {}ms", H::Chrono::milliseconds_f{ this->lastSeekPosition }.count());
	hr = MFTools::SourceReader::SetPosition(this->sourceReader, this->lastSeekPosition);
	LOG_FAILED(hr);

	if (auto activeVideoStream = avSourceStreamManager->GetActiveVideoStream()) {
		activeVideoStream->ClearSamples();
		this->RequestNextSampleInternal(avSourceStreamManager, AvStreamType::Video);
	}

	if (auto activeAudioStream = avSourceStreamManager->GetActiveAudioStream()) {
		activeAudioStream->ClearSamples();
		this->RequestNextSampleInternal(avSourceStreamManager, AvStreamType::Audio);
	}

	return S_OK;
}



//
// Source reader handlers
//
AvReader::ReaderCallback::ReaderCallback(AvReader* parent)
	: parent(parent)
{
}

AvReader::ReaderCallback::~ReaderCallback() {
}

STDMETHODIMP AvReader::ReaderCallback::OnReadSample(
	_In_  HRESULT hrStatus,
	_In_  DWORD dwStreamIndex,
	_In_  DWORD dwStreamFlags,
	_In_  LONGLONG llTimestamp,
	_In_opt_  IMFSample* pSample)
{
	HRESULT hr = S_OK;

	try {
		hr = this->parent->OnReadSampleAsync(
			hrStatus,
			dwStreamIndex,
			dwStreamFlags,
			llTimestamp,
			pSample);
	}
	catch (...) {
		hr = E_FAIL;
	}

	return hr;
}

STDMETHODIMP AvReader::ReaderCallback::OnFlush(
	_In_  DWORD dwStreamIndex)
{
	HRESULT hr = S_OK;

	try {
		hr = this->parent->OnFlushAsync(dwStreamIndex);
	}
	catch (...) {
		hr = E_FAIL;
	}

	return hr;
}

STDMETHODIMP AvReader::ReaderCallback::OnEvent(
	_In_  DWORD dwStreamIndex,
	_In_  IMFMediaEvent* pEvent)
{
	HRESULT hr = S_OK;
	return hr;
}