#pragma once
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <mferror.h>
#include <wrl.h>
#include <shlwapi.h>
#include <string>
#include <libhelpers/HSystem.h>
#include "MediaFormat/MediaFormat.h"

class ChunkMerger {
public:
	ChunkMerger(IMFByteStream* outputStream,
		Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioIn, Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioOut,
		Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoIn, Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoOut,
		const IVideoCodecSettings* settings, std::vector<std::wstring>&& filesToMerge, std::wstring containerExt,
		MediaContainerType containerType, bool tryRemux);

	// Can throw exception when result file can be broken
	// Can throw FinalizedWithWarningException when file is working but merge operation was not fully finished
	void Merge();
	
private:
	Microsoft::WRL::ComPtr<IMFMediaType> CreateVideoOutMediaType();

private:
	bool useVideoStream = true;
	bool useAudioStream = true;

	const IVideoCodecSettings* settings;
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioIn;
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioOut;
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoIn;
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoOut;

	LONGLONG videoHns = 0;
	LONGLONG audioHns = 0;
	LONGLONG audioSamples = 0;
	UINT32 sampleRate = 0;
	//https://stackoverflow.com/questions/33401149/ffmpeg-adding-0-05-seconds-of-silence-to-transcoded-aac-file
	int firstSamplesCount = 2048;

	std::vector<std::wstring> filesToMerge;
	Microsoft::WRL::ComPtr<IMFSinkWriter> writer;

	DWORD videoStreamIndexToWrite = (DWORD)-1;
	DWORD audioStreamIndexToWrite = (DWORD)-1;

	bool audioRemuxUsed = false;

	bool WriteInner(Microsoft::WRL::ComPtr<IMFSinkWriter> writerArg, Microsoft::WRL::ComPtr<IMFSourceReader> reader, DWORD readFrom, DWORD writeTo, bool audio);

	bool TryInitVideoRemux(IMFSourceReader* chunkReader);
	bool TryInitAudioRemux(IMFSourceReader* chunkReader, MediaContainerType containerType);

	static void SetIMFMediaTypeItem(IMFMediaType* dst, IMFMediaType* src, const GUID& key);

	static Microsoft::WRL::ComPtr<IMFSourceReader> CreateSourceReader(const std::wstring& filePath);
};