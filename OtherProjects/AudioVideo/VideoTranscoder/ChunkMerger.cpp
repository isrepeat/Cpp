#include "ChunkMerger.h"
#include <Helpers/System.h>

// NOTE: Merging takes longer than just writing bytes into a stream. Maybe optimize somehow? (try use GetNativeMediaType without decoding)
ChunkMerger::ChunkMerger(const std::wstring& outputPath,
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioIn, Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioOut,
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoIn, Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoOut,
	std::vector<std::wstring>&& filesToMerge)
	: mediaTypeAudioIn{ mediaTypeAudioIn }
	, mediaTypeAudioOut{ mediaTypeAudioOut }
	, mediaTypeVideoIn{ mediaTypeVideoIn }
	, mediaTypeVideoOut{ mediaTypeVideoOut }
	, filesToMerge{ filesToMerge }
{
	Microsoft::WRL::ComPtr<IMFSourceReader> reader;
	HRESULT hr = MFCreateSourceReaderFromURL(filesToMerge[0].c_str(), nullptr, reader.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	hr = MFCreateSinkWriterFromURL(outputPath.c_str(), nullptr, nullptr, writer.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	Microsoft::WRL::ComPtr<IMFMediaType> srcMediaTypeAudio;
	hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, srcMediaTypeAudio.GetAddressOf());
	if (hr != MF_E_INVALIDSTREAMNUMBER) {
		H::System::ThrowIfFailed(hr);
	}
	Microsoft::WRL::ComPtr<IMFMediaType> srcMediaTypeVideo;
	hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, srcMediaTypeVideo.GetAddressOf());
	if (hr != MF_E_INVALIDSTREAMNUMBER) {
		H::System::ThrowIfFailed(hr);
	}


	if (srcMediaTypeAudio) {
		useAudioStream = true;

		hr = writer->AddStream(srcMediaTypeAudio.Get(), &audioStreamIndexToWrite);
		H::System::ThrowIfFailed(hr);

		hr = writer->SetInputMediaType(audioStreamIndexToWrite, srcMediaTypeAudio.Get(), nullptr);
		H::System::ThrowIfFailed(hr);



		//hr = writer->AddStream(this->mediaTypeAudioOut.Get(), &audioStreamIndexToWrite);
		//H::System::ThrowIfFailed(hr);

		//mediaTypeAudioBetween = mediaTypeAudioIn;
		//hr = MFCreateMediaType(mediaTypeAudioBetween.ReleaseAndGetAddressOf());
		//H::System::ThrowIfFailed(hr);

		//hr = mediaTypeAudioBetween->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
		//H::System::ThrowIfFailed(hr);

		//hr = mediaTypeAudioBetween->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
		//H::System::ThrowIfFailed(hr);

		//hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, mediaTypeAudioBetween.Get());
		//H::System::ThrowIfFailed(hr);



		//hr = writer->AddStream(this->mediaTypeAudioOut.Get(), &audioStreamIndexToWrite);
		//H::System::ThrowIfFailed(hr);

		//hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, this->mediaTypeAudioIn.Get());
		//H::System::ThrowIfFailed(hr);

		//Microsoft::WRL::ComPtr<IMFMediaType> audioMediaTypeForWriter;
		//hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, audioMediaTypeForWriter.GetAddressOf());
		//H::System::ThrowIfFailed(hr);

		//hr = writer->SetInputMediaType(audioStreamIndexToWrite, audioMediaTypeForWriter.Get(), nullptr);
		//H::System::ThrowIfFailed(hr);
	}

	if (srcMediaTypeVideo) {
		useVideoStream = true;

		//hr = writer->AddStream(this->mediaTypeVideoOut.Get(), &videoStreamIndexToWrite);
		hr = writer->AddStream(srcMediaTypeVideo.Get(), &videoStreamIndexToWrite);
		H::System::ThrowIfFailed(hr);

		//hr = writer->SetInputMediaType(videoStreamIndexToWrite, this->mediaTypeVideoIn.Get(), NULL);
		hr = writer->SetInputMediaType(videoStreamIndexToWrite, srcMediaTypeVideo.Get(), NULL);
		H::System::ThrowIfFailed(hr);



		//hr = writer->AddStream(srcMediaTypeVideo.Get(), &videoStreamIndexToWrite);
		//H::System::ThrowIfFailed(hr);

		//hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, srcMediaTypeVideo.Get());
		//H::System::ThrowIfFailed(hr);

		//Microsoft::WRL::ComPtr<IMFMediaType> videoMediaTypeForWriter;
		//hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, videoMediaTypeForWriter.GetAddressOf());
		//H::System::ThrowIfFailed(hr);

		//hr = writer->SetInputMediaType(videoStreamIndexToWrite, videoMediaTypeForWriter.Get(), NULL);
		//H::System::ThrowIfFailed(hr);



		//hr = writer->AddStream(this->mediaTypeVideoOut.Get(), &videoStreamIndexToWrite);
		//H::System::ThrowIfFailed(hr);

		//hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, this->mediaTypeVideoIn.Get());
		//H::System::ThrowIfFailed(hr);

		//Microsoft::WRL::ComPtr<IMFMediaType> videoMediaTypeForWriter;
		//hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, videoMediaTypeForWriter.GetAddressOf());
		//H::System::ThrowIfFailed(hr);

		//hr = writer->SetInputMediaType(videoStreamIndexToWrite, videoMediaTypeForWriter.Get(), NULL);
		//H::System::ThrowIfFailed(hr);



		//hr = writer->AddStream(this->mediaTypeVideoOut.Get(), &videoStreamIndexToWrite);
		//H::System::ThrowIfFailed(hr);

		//hr = writer->SetInputMediaType(videoStreamIndexToWrite, this->mediaTypeVideoIn.Get(), NULL);
		//H::System::ThrowIfFailed(hr);
	}

	hr = writer->BeginWriting();
	H::System::ThrowIfFailed(hr);
}


void ChunkMerger::Merge() {
	try {
		for (auto& file : filesToMerge) {
			Microsoft::WRL::ComPtr<IMFSourceReader> reader;
			HRESULT hr = MFCreateSourceReaderFromURL(file.c_str(), nullptr, reader.GetAddressOf());
			H::System::ThrowIfFailed(hr);

			//if (useAudioStream) {
			//	hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, mediaTypeAudioIn.Get());
			//	H::System::ThrowIfFailed(hr);
			//}

			//if (useVideoStream) {
			//	hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, mediaTypeVideoIn.Get());
			//	//hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, mediaTypeVideoBetween.Get());
			//	H::System::ThrowIfFailed(hr);
			//}


			bool videoDone = false;
			bool audioDone = false;

			while (true) {
				if (useVideoStream && useAudioStream) {
					videoDone = WriteInner(writer, reader, MF_SOURCE_READER_FIRST_VIDEO_STREAM, videoStreamIndexToWrite, false);

					while ((audioHns < videoHns && !audioDone) || (videoDone && !audioDone)) {
						audioDone = WriteInner(writer, reader, MF_SOURCE_READER_FIRST_AUDIO_STREAM, audioStreamIndexToWrite, true);
					}
					if (videoDone && audioDone) {
						firstSamplesCount = 2048;
						break;
					}
				}
				else if (useAudioStream) {
					while (!audioDone) {
						audioDone = WriteInner(writer, reader, MF_SOURCE_READER_FIRST_AUDIO_STREAM, audioStreamIndexToWrite, true);
					}

					firstSamplesCount = 2048;
					break;
				}
				else if (useVideoStream) {
					videoDone = WriteInner(writer, reader, MF_SOURCE_READER_FIRST_VIDEO_STREAM, videoStreamIndexToWrite, false);
					if (videoDone)
						break;
				}
			}
		}

		auto hr = writer->Finalize();
		if (hr != MF_E_SINK_NO_SAMPLES_PROCESSED) { // occured when was called BeginWritting but not calls WriteSample yet
			H::System::ThrowIfFailed(hr);
		}
	}
	catch (H::System::ComException& ex) {
		// If was error during merging try end writing with already handled chunks
		if (ex.ErrorCode() == MF_E_INVALIDSTREAMNUMBER) {
			auto hr = writer->Finalize();
			H::System::ThrowIfFailed(hr);
		}
		else {
			throw;
		}
	}
}

bool ChunkMerger::WriteInner(Microsoft::WRL::ComPtr<IMFSinkWriter> writer, Microsoft::WRL::ComPtr<IMFSourceReader> reader, DWORD readFrom, DWORD writeTo, bool audio) {
	// NOTE: If not enaugh space on disk where merging chunks may be 0xC00D36B3: MF_E_INVALIDSTREAMNUMBER
	HRESULT hr = reader->SetStreamSelection(readFrom, true);
	H::System::ThrowIfFailed(hr);

	Microsoft::WRL::ComPtr<IMFSample> sample;
	DWORD stream;
	LONGLONG pts;

	hr = reader->ReadSample(readFrom, 0, nullptr, &stream, &pts, sample.GetAddressOf());
	if (FAILED(hr) || (sample == nullptr && stream == MF_SOURCE_READERF_ENDOFSTREAM))
		return true;


	LONGLONG duration;

	hr = sample->GetSampleDuration(&duration);
	H::System::ThrowIfFailed(hr);

	if (audio) {
		if (firstSamplesCount > 0) {
			Microsoft::WRL::ComPtr<IMFMediaBuffer> mediaBuffer;
			hr = sample->ConvertToContiguousBuffer(&mediaBuffer);
			DWORD len;
			hr = mediaBuffer->GetCurrentLength(&len);

			firstSamplesCount -= len / 2;
			return false;
		}
		hr = sample->SetSampleTime(audioHns);
		audioHns += duration;
	}
	else {
		hr = sample->SetSampleTime(videoHns);
		videoHns += duration;
	}
	H::System::ThrowIfFailed(hr);
	hr = writer->WriteSample(writeTo, sample.Get());
	if (FAILED(hr))
		return true;

	return false;
}