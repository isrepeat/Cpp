#include "ChunkMerger.h"
#include <libhelpers/HMathCP.h>
#include <libhelpers/HTime.h>
#include <libhelpers/UniquePROPVARIANT.h>
#include "MediaFormat/MediaFormatCodecsSupport.h"
#include "Platform/PlatformClassFactory.h"
#include "FinalizedWithWarningException.h"
#if SPDLOG_ENABLED
#include <spdlog/LoggerWrapper.h>
#endif

// NOTE: Merging takes longer than just writing bytes into a stream. Maybe optimize somehow?
ChunkMerger::ChunkMerger(IMFByteStream* outputStream,
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioIn, Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioOut,
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoIn, Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoOut,
	const IVideoCodecSettings* settings, std::vector<std::wstring>&& filesToMerge, std::wstring containerExt,
	MediaContainerType containerType, bool tryRemux)
	: mediaTypeAudioIn{ mediaTypeAudioIn }
	, mediaTypeAudioOut{ mediaTypeAudioOut }
	, mediaTypeVideoIn{ mediaTypeVideoIn }
	//, mediaTypeVideoOut{ mediaTypeVideoOut } // not use for now, because merging HEVC -> HEVC was failed
	, filesToMerge{ filesToMerge }
	, settings{ settings }
{
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<IMFSourceReader> reader = ChunkMerger::CreateSourceReader(filesToMerge[0]);

	Microsoft::WRL::ComPtr<IMFAttributes> writerAttr;

	hr = MFCreateAttributes(writerAttr.GetAddressOf(), 1);
	H::System::ThrowIfFailed(hr);

	// must be used if remux will be enabled
	// remux will deliver very fast so that throttling will be enabled
	// https://learn.microsoft.com/en-us/windows/win32/medfound/mf-sink-writer-disable-throttling
	hr = writerAttr->SetUINT32(MF_SINK_WRITER_DISABLE_THROTTLING, TRUE);
	H::System::ThrowIfFailed(hr);

	Microsoft::WRL::ComPtr<IMFByteStream> byteStream = outputStream;
	hr = MFCreateSinkWriterFromURL(containerExt.c_str(), byteStream.Get(), writerAttr.Get(), writer.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	if (mediaTypeAudioOut == nullptr) {
		useAudioStream = false;
	}
	if (mediaTypeVideoOut == nullptr) {
		useVideoStream = false;
	}

	if (useAudioStream) {
		if (!tryRemux || !this->TryInitAudioRemux(reader.Get(), containerType)) {
			hr = writer->AddStream(this->mediaTypeAudioOut.Get(), &audioStreamIndexToWrite);
			H::System::ThrowIfFailed(hr);

			hr = reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, this->mediaTypeAudioIn.Get());
			H::System::ThrowIfFailed(hr);

			Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioForWriter;
			hr = reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, mediaTypeAudioForWriter.GetAddressOf());
			H::System::ThrowIfFailed(hr);

			hr = writer->SetInputMediaType(audioStreamIndexToWrite, mediaTypeAudioForWriter.Get(), nullptr);
			H::System::ThrowIfFailed(hr);
		}
	}
	if (useVideoStream) {
		// HEVC TRANSCODING WORKAROUND:
		// Incoming video must be encoded in H264 for always types, otherwise we can't mrege form HEVC to HEVC,
		// but can H264 -> HEVC or H264 -> H264.
		if (!tryRemux || !this->TryInitVideoRemux(reader.Get())) {
			if (containerType == MediaContainerType::MP4) {
				this->mediaTypeVideoOut = CreateVideoOutMediaType();
			}
			else {
				this->mediaTypeVideoOut = mediaTypeVideoOut;
			}

			hr = writer->AddStream(this->mediaTypeVideoOut.Get(), &videoStreamIndexToWrite);
			H::System::ThrowIfFailed(hr);

			hr = reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, this->mediaTypeVideoIn.Get());
			H::System::ThrowIfFailed(hr);

			Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoForWriter;
			hr = reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, mediaTypeVideoForWriter.GetAddressOf());
			H::System::ThrowIfFailed(hr);

			hr = writer->SetInputMediaType(videoStreamIndexToWrite, mediaTypeVideoForWriter.Get(), NULL);
			H::System::ThrowIfFailed(hr);
		}
	}

	hr = writer->BeginWriting();
	H::System::ThrowIfFailed(hr);
}



Microsoft::WRL::ComPtr<IMFMediaType> ChunkMerger::CreateVideoOutMediaType()
{
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<IMFMediaType> mediaType;

	hr = MFCreateMediaType(mediaType.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	H::System::ThrowIfFailed(hr);

	auto codecSup = MediaFormatCodecsSupport::Instance();
	auto vcodecId = codecSup->MapVideoCodec(settings->GetCodecType());

	hr = mediaType->SetGUID(MF_MT_SUBTYPE, vcodecId);
	H::System::ThrowIfFailed(hr);

	// default profile of H264 can fail on sink->Finalize with video bitrate > 80 mbits.
	//uint32_t avgVideoBitrate = (std::min)(videoParams->AvgBitrate, uint32_t(80 * 1000000));

	hr = mediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
	H::System::ThrowIfFailed(hr);

	hr = MFSetAttributeRatio(mediaType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
	H::System::ThrowIfFailed(hr);

	auto basicSettings = settings->GetBasicSettings();

	if (basicSettings) {
		hr = mediaType->SetUINT32(MF_MT_AVG_BITRATE, basicSettings->bitrate);
		H::System::ThrowIfFailed(hr);

		uint32_t testBitrate;
		hr = mediaType->GetUINT32(MF_MT_AVG_BITRATE, &testBitrate);
		H::System::ThrowIfFailed(hr);

		hr = MFSetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, basicSettings->width, basicSettings->height);
		H::System::ThrowIfFailed(hr);

		hr = MFSetAttributeRatio(mediaType.Get(), MF_MT_FRAME_RATE, basicSettings->fps, 1);
		H::System::ThrowIfFailed(hr);
	}

	return mediaType;
}


void ChunkMerger::Merge() {
	HRESULT hr = S_OK;
	// if true then Finalize == S_OK but when merging chunks in try{} there was exception(maybe in WriteInner)
	bool finalizedWithWarning = false;
#if SPDLOG_ENABLED
	LOG_DEBUG("ChunkMerger::Merge started merging chunks");
#endif
	try {
		for (const auto& file : filesToMerge) {
			Microsoft::WRL::ComPtr<IMFSourceReader> reader = ChunkMerger::CreateSourceReader(file);

			if (useAudioStream) {
				hr = reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, mediaTypeAudioIn.Get());
				H::System::ThrowIfFailed(hr);
			}

			if (useVideoStream) {
				hr = reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, mediaTypeVideoIn.Get());
				H::System::ThrowIfFailed(hr);
			}

			bool videoDone = false;
			bool audioDone = false;

			while (true) {
				if (useVideoStream && useAudioStream) {
					videoDone = WriteInner(writer, reader, (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, videoStreamIndexToWrite, false);

					while ((audioHns < videoHns && !audioDone) || (videoDone && !audioDone)) {
						audioDone = WriteInner(writer, reader, (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, audioStreamIndexToWrite, true);
					}
					if (videoDone && audioDone) {
						if (!this->audioRemuxUsed) {
							firstSamplesCount = 2048;
						}
						break;
					}
				}
				else if (useAudioStream) {
					while (!audioDone) {
						audioDone = WriteInner(writer, reader, (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, audioStreamIndexToWrite, true);
					}

					if (!this->audioRemuxUsed) {
						firstSamplesCount = 2048;
					}
					break;
				}
				else if (useVideoStream) {
					videoDone = WriteInner(writer, reader, (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, videoStreamIndexToWrite, false);
					if (videoDone)
						break;
				}
			}
		}
	}
	catch (const HResultException& ex) {
		// If was error during merging try end writing with already handled chunks
		if (ex.GetHRESULT() == MF_E_INVALIDSTREAMNUMBER
			|| ex.GetHRESULT() == HRESULT_FROM_WIN32(ERROR_DISK_FULL) // when the disk is full file may be finalized successfully
			)
		{
			finalizedWithWarning = true;
		}
		else {
#if SPDLOG_ENABLED
			LOG_ERROR("ChunkMerger::Merge exception thrown: {}", ex.GetHRESULT());
#endif
			throw;
		}
	}

	hr = writer->Finalize();
	if (FAILED(hr) && hr != MF_E_SINK_NO_SAMPLES_PROCESSED) { // occured when was called BeginWritting but not calls WriteSample yet
		H::System::ThrowIfFailed(hr);
	}

	if (finalizedWithWarning) {
#if SPDLOG_ENABLED
		LOG_ERROR("ChunkMerger::Merge exception during finalization");
#endif
		throw FinalizedWithWarningException();
	}
}

bool ChunkMerger::WriteInner(Microsoft::WRL::ComPtr<IMFSinkWriter> writerArg, Microsoft::WRL::ComPtr<IMFSourceReader> reader, DWORD readFrom, DWORD writeTo, bool audio) {
	// NOTE: If not enaugh space on disk where merging chunks may be 0xC00D36B3: MF_E_INVALIDSTREAMNUMBER
	HRESULT hr = reader->SetStreamSelection(readFrom, true);
	H::System::ThrowIfFailed(hr);

	Microsoft::WRL::ComPtr<IMFSample> sample;
	DWORD stream;
	LONGLONG pts;

	hr = reader->ReadSample(readFrom, 0, nullptr, &stream, &pts, sample.GetAddressOf());
	if (FAILED(hr) || (sample == nullptr && stream == MF_SOURCE_READERF_ENDOFSTREAM))
		return true;

	Microsoft::WRL::ComPtr<IMFSample> sampleToWrite;

	// create new sample because initial sample may contain additional data that prevents writer to work correctly
	hr = MFCreateSample(sampleToWrite.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	LONGLONG duration;

	hr = sample->GetSampleDuration(&duration);
	H::System::ThrowIfFailed(hr);

	hr = sampleToWrite->SetSampleDuration(duration);
	H::System::ThrowIfFailed(hr);

	if (audio) {
		if (firstSamplesCount > 0) {
			Microsoft::WRL::ComPtr<IMFMediaBuffer> mediaBuffer;
			hr = sample->ConvertToContiguousBuffer(&mediaBuffer);
			H::System::ThrowIfFailed(hr);

			DWORD len;
			hr = mediaBuffer->GetCurrentLength(&len);
			H::System::ThrowIfFailed(hr);

			firstSamplesCount -= len / 2;
			return false;
		}

		hr = sampleToWrite->SetSampleTime(audioHns);
		H::System::ThrowIfFailed(hr);

		audioHns += duration;
	}
	else {
		hr = sampleToWrite->SetSampleTime(videoHns);
		H::System::ThrowIfFailed(hr);

		videoHns += duration;
	}

	DWORD bufCount = 0;

	hr = sample->GetBufferCount(&bufCount);
	H::System::ThrowIfFailed(hr);

	for (DWORD i = 0; i < bufCount; ++i) {
		Microsoft::WRL::ComPtr<IMFMediaBuffer> buf;

		hr = sample->GetBufferByIndex(i, buf.GetAddressOf());
		H::System::ThrowIfFailed(hr);

		hr = sampleToWrite->AddBuffer(buf.Get());
		H::System::ThrowIfFailed(hr);
	}
	
	hr = writerArg->WriteSample(writeTo, sampleToWrite.Get());
	// allow throw errors. For example disk full
	H::System::ThrowIfFailed(hr);

	return false;
}

bool ChunkMerger::TryInitVideoRemux(IMFSourceReader* chunkReader) {
	try {
		HRESULT hr = S_OK;
		Microsoft::WRL::ComPtr<IMFMediaType> nativeType;

		hr = chunkReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, (DWORD)MF_SOURCE_READER_CURRENT_TYPE_INDEX, nativeType.GetAddressOf());
		H::System::ThrowIfFailed(hr);

		GUID subtype = {};
		hr = nativeType->GetGUID(MF_MT_SUBTYPE, &subtype);
		H::System::ThrowIfFailed(hr);

		if (subtype != MFVideoFormat_H264 && subtype != MFVideoFormat_HEVC) {
			return false;
		}

		Microsoft::WRL::ComPtr<IMFMediaType> writerMediaType;

		hr = MFCreateMediaType(writerMediaType.GetAddressOf());
		H::System::ThrowIfFailed(hr);

		ChunkMerger::SetIMFMediaTypeItem(writerMediaType.Get(), nativeType.Get(), MF_MT_MAJOR_TYPE);
		ChunkMerger::SetIMFMediaTypeItem(writerMediaType.Get(), nativeType.Get(), MF_MT_SUBTYPE);
		ChunkMerger::SetIMFMediaTypeItem(writerMediaType.Get(), nativeType.Get(), MF_MT_INTERLACE_MODE);
		ChunkMerger::SetIMFMediaTypeItem(writerMediaType.Get(), nativeType.Get(), MF_MT_FRAME_RATE);
		ChunkMerger::SetIMFMediaTypeItem(writerMediaType.Get(), nativeType.Get(), MF_MT_FRAME_SIZE);

		Microsoft::WRL::ComPtr<IMFMediaType> readerMediaType;

		hr = MFCreateMediaType(readerMediaType.GetAddressOf());
		H::System::ThrowIfFailed(hr);

		hr = writerMediaType->CopyAllItems(readerMediaType.Get());
		H::System::ThrowIfFailed(hr);

		hr = readerMediaType->DeleteItem(MF_MT_FRAME_RATE);
		H::System::ThrowIfFailed(hr);

		hr = chunkReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, readerMediaType.Get());
		H::System::ThrowIfFailed(hr);

		hr = writer->AddStream(writerMediaType.Get(), &videoStreamIndexToWrite);
		H::System::ThrowIfFailed(hr);

		hr = writer->SetInputMediaType(videoStreamIndexToWrite, writerMediaType.Get(), NULL);
		H::System::ThrowIfFailed(hr);

		this->mediaTypeVideoIn = readerMediaType;
		this->mediaTypeVideoOut = writerMediaType;
	}
	catch (...) {
		return false;
	}

	return true;
}

bool ChunkMerger::TryInitAudioRemux(IMFSourceReader* chunkReader, MediaContainerType containerType) {
	if (containerType == MediaContainerType::WMV) {
		// for wmv+aac will be sample->GetSampleDuration == MF_E_NO_SAMPLE_DURATION
		return false;
	}

	try {
		HRESULT hr = S_OK;
		Microsoft::WRL::ComPtr<IMFMediaType> nativeType;

		// cycle through native media types to get supported type
		// ALAC has more than 1 native types
		for (DWORD i = 0;; ++i) {
			hr = chunkReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, i, nativeType.ReleaseAndGetAddressOf());
			if (hr == MF_E_NO_MORE_TYPES) {
				break;
			}
			H::System::ThrowIfFailed(hr);

			GUID subtype = {};
			hr = nativeType->GetGUID(MF_MT_SUBTYPE, &subtype);
			H::System::ThrowIfFailed(hr);

			if (subtype != MFAudioFormat_AAC && subtype != MFAudioFormat_MP3 && subtype != MFAudioFormat_Dolby_AC3 && subtype != MFAudioFormat_ALAC) {
				nativeType.Reset();
				continue;
			}

			break;
		}

		if (!nativeType) {
			return false;
		}

		Microsoft::WRL::ComPtr<IMFMediaType> readerMediaType;

		hr = MFCreateMediaType(readerMediaType.GetAddressOf());
		H::System::ThrowIfFailed(hr);

		ChunkMerger::SetIMFMediaTypeItem(readerMediaType.Get(), nativeType.Get(), MF_MT_MAJOR_TYPE);
		ChunkMerger::SetIMFMediaTypeItem(readerMediaType.Get(), nativeType.Get(), MF_MT_SUBTYPE);

		hr = chunkReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, readerMediaType.Get());
		H::System::ThrowIfFailed(hr);

		hr = writer->AddStream(nativeType.Get(), &audioStreamIndexToWrite);
		H::System::ThrowIfFailed(hr);

		hr = writer->SetInputMediaType(audioStreamIndexToWrite, nativeType.Get(), nullptr);
		H::System::ThrowIfFailed(hr);

		this->mediaTypeAudioIn = readerMediaType;
		this->mediaTypeAudioOut = nativeType;

		this->audioRemuxUsed = true;
		this->firstSamplesCount = 0;
	}
	catch (...) {
		return false;
	}

	return true;
}

void ChunkMerger::SetIMFMediaTypeItem(IMFMediaType* dst, IMFMediaType* src, const GUID& key) {
	HRESULT hr = S_OK;
	UniquePROPVARIANT pv;

	hr = src->GetItem(key, pv.get());
	H::System::ThrowIfFailed(hr);

	hr = dst->SetItem(key, *pv.get());
	H::System::ThrowIfFailed(hr);
}

Microsoft::WRL::ComPtr<IMFSourceReader> ChunkMerger::CreateSourceReader(const std::wstring& filePath) {
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<IMFAttributes> readerAttr;

	hr = MFCreateAttributes(readerAttr.GetAddressOf(), 2);
	H::System::ThrowIfFailed(hr);

	hr = readerAttr->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
	H::System::ThrowIfFailed(hr);

	// Use MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING to get nv12 samples without vertical alignment
	// Same problem discussed here https://learn.microsoft.com/en-us/answers/questions/804470/unexpected-u-v-plane-offset-with-windows-media-fou
	// Also some possible explanation is description of V4L2_PIX_FMT_NV12_16L16 https://docs.kernel.org/userspace-api/media/v4l/pixfmt-yuv-planar.html
	// V4L2_PIX_FMT_NV12_16L16 stores pixels in 16x16 tiles, and stores tiles linearly in memory. The line stride and image height must be aligned to a multiple of 16. The layouts of the luma and chroma planes are identical.
	// "image height must be aligned to a multiple of 16" looks like that vertical alignment, but why MediaFoundation does so without MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING is unknown for now
	hr = readerAttr->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE);
	H::System::ThrowIfFailed(hr);

	Microsoft::WRL::ComPtr<IMFSourceReader> reader;
	hr = MFCreateSourceReaderFromURL(filePath.c_str(), readerAttr.Get(), reader.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	return reader;
}
