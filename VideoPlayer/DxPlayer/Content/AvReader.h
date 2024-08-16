#pragma once
#include "../Common/Config.h"
#if ENGINE_TYPE == MEDIA_ENDIGNE_LOGIC
#include <Helpers/MediaFoundation/MFUser.h>
#include <Helpers/MediaFoundation/MFLibs.h>
#include <Helpers/ThreadSafeObject.hpp>

#include "AvSourceStreamManager.h"
#include "AvReaderDxgiManager.h"
#include "AvReaderDxgiEffect.h"
//#include "RewindParams.h"


class AvReader : public MFUser {
public:
    using _AvSourceStreamManagerSafeObj = H::ThreadSafeObject<std::mutex, std::unique_ptr<AvSourceStreamManager>>;

    AvReader(IStream* stream, H::Dx::DxDeviceSafeObj* dxDeviceSafeObj);
    ~AvReader();

    H::Chrono::Hns GetSourceDuration();

    _AvSourceStreamManagerSafeObj::_Locked GetLockedAvSourceStreamManager();

    //void SeekAsync(AvReaderRewindParams rewindParams);

    void RequestNeccessaryVideoSamples();
    void RequestNeccessaryAudioSamples();

    std::unique_ptr<MF::MFVideoSample> AvReader::PopNextVideoSample();
    std::unique_ptr<MF::MFAudioSample> AvReader::PopNextAudioSample();

private:
    void InitReader(IStream* stream);
    HRESULT ConfigureDecoders();
    DWORD GetStreamCountFromReaderInternal();

    HRESULT RequestNextSampleInternal(_AvSourceStreamManagerSafeObj::_Locked& avSourceStreamManager, AvStreamType streamType);
    HRESULT ProcessVideoSample(std::unique_ptr<MF::MFSample> mfVideoSample);
    HRESULT ProcessAudioSample(std::unique_ptr<MF::MFSample> mfAudioSample);

    void ClearStream(_AvSourceStreamManagerSafeObj::_Locked& avSourceStreamManager, DWORD dwStreamIndex);

    HRESULT OnReadSampleAsync(
        _In_  HRESULT hrStatus,
        _In_  DWORD dwStreamIndex,
        _In_  DWORD dwStreamFlags,
        _In_  LONGLONG llTimestamp,
        _In_opt_  IMFSample* pSample);

    HRESULT OnFlushAsync(DWORD dwStreamIndex);

private:
    class ReaderCallback :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::ClassicCom>,
        IMFSourceReaderCallback>
    {
    public:
        ReaderCallback(AvReader* parent);
        virtual ~ReaderCallback();

        STDMETHOD(OnReadSample(
            _In_  HRESULT hrStatus,
            _In_  DWORD dwStreamIndex,
            _In_  DWORD dwStreamFlags,
            _In_  LONGLONG llTimestamp,
            _In_opt_  IMFSample* pSample)) override;

        STDMETHOD(OnFlush(
            _In_  DWORD dwStreamIndex)) override;

        STDMETHOD(OnEvent(
            _In_  DWORD dwStreamIndex,
            _In_  IMFMediaEvent* pEvent)) override;

    private:
        AvReader* parent;
    };


private:
    std::mutex mx;
    Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
    Microsoft::WRL::ComPtr<ReaderCallback> readerCallback;
    std::unique_ptr<AvReaderDxgiManager> avReaderDxgiManager;

    //
    // Stream related members
    //
    std::unique_ptr<_AvSourceStreamManagerSafeObj> avSourceStreamManagerSafeObj;

    const DWORD firstVideoStreamIdx = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
    const DWORD firstAudioStreamIdx = MF_SOURCE_READER_FIRST_AUDIO_STREAM;

    DWORD videoStreamIdx = -1;
    DWORD audioStreamIdx = -1;

    bool hasVideo = false;
    bool hasAudio = false;

    //
    // Playback
    //
    //std::weak_ptr<AvReaderRewindAsyncResult> rewindAsyncResultWeak;
    H::Chrono::Hns lastSeekPosition = 0_hns;
    H::Chrono::Hns sourceDuration = 0_hns;
};

#elif ENGINE_TYPE == DX_PLAYER_RENDER

#include <Helpers/MediaFoundation/MFUser.h>
#include <Helpers/MediaFoundation/MFLibs.h>
#include <Helpers/ThreadSafeObject.hpp>

#include "AvSourceStreamManager.h"
#include "AvReaderDxgiManager.h"
#include "AvReaderDxgiEffect.h"


class AvReader : public MFUser {
public:
	using _AvSourceStreamManagerSafeObj = H::ThreadSafeObject<std::mutex, std::unique_ptr<AvSourceStreamManager>>;
    
    AvReader(IStream* stream, const std::shared_ptr<DX::DeviceResources>& deviceResources);

    void RequestNeccessaryVideoSamples();
    void RequestNeccessaryAudioSamples();

    std::unique_ptr<MF::MFVideoSample> AvReader::PopNextVideoSample();
    std::unique_ptr<MF::MFAudioSample> AvReader::PopNextAudioSample();

private:
	void InitReader(IStream* stream);
    HRESULT ConfigureDecoders();
    DWORD GetStreamCountFromReaderInternal();
	
    HRESULT RequestNextSampleInternal(_AvSourceStreamManagerSafeObj::_Locked& avSourceStreamManager, AvStreamType streamType);
    HRESULT ProcessVideoSample(std::unique_ptr<MF::MFSample> mfVideoSample);
    HRESULT ProcessAudioSample(std::unique_ptr<MF::MFSample> mfAudioSample);

	HRESULT OnReadSampleAsync(
		_In_  HRESULT hrStatus,
		_In_  DWORD dwStreamIndex,
		_In_  DWORD dwStreamFlags,
		_In_  LONGLONG llTimestamp,
		_In_opt_  IMFSample* pSample);

	HRESULT OnFlushAsync(DWORD dwStreamIndex);

private:
    class ReaderCallback :
        public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::ClassicCom>,
        IMFSourceReaderCallback>
    {
    public:
        ReaderCallback(AvReader* parent);
        virtual ~ReaderCallback();

        STDMETHOD(OnReadSample(
            _In_  HRESULT hrStatus,
            _In_  DWORD dwStreamIndex,
            _In_  DWORD dwStreamFlags,
            _In_  LONGLONG llTimestamp,
            _In_opt_  IMFSample* pSample)) override;

        STDMETHOD(OnFlush(
            _In_  DWORD dwStreamIndex)) override;

        STDMETHOD(OnEvent(
            _In_  DWORD dwStreamIndex,
            _In_  IMFMediaEvent* pEvent)) override;

    private:
        AvReader* parent;
    };


private:
    std::mutex mx;
    Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
    Microsoft::WRL::ComPtr<ReaderCallback> readerCallback;
    std::unique_ptr<AvReaderDxgiManager> avReaderDxgiManager;

    //
    // Stream related members
    //
    std::unique_ptr<_AvSourceStreamManagerSafeObj> avSourceStreamManagerSafeObj;

    const DWORD firstVideoStreamIdx = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
    const DWORD firstAudioStreamIdx = MF_SOURCE_READER_FIRST_AUDIO_STREAM;

    DWORD videoStreamIdx = -1;
    DWORD audioStreamIdx = -1;

    bool hasVideo = false;
    bool hasAudio = false;
};
#endif