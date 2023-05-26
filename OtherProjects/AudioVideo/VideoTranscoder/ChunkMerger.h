#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <wrl.h>
#include "MFInclude.h"

class ChunkMerger {
public:
	ChunkMerger(const std::wstring& outputPath,
		Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioIn, Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioOut,
		Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoIn, Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoOut,
		std::vector<std::wstring>&& filesToMerge);

	void Merge();

private:
	bool useVideoStream = false;
	bool useAudioStream = false;

	LONGLONG videoHns = 0;
	LONGLONG audioHns = 0;
	LONGLONG audioSamples = 0;
	UINT32 sampleRate = 0;
	//https://stackoverflow.com/questions/33401149/ffmpeg-adding-0-05-seconds-of-silence-to-transcoded-aac-file
	int firstSamplesCount = 2048;

	std::vector<std::wstring> filesToMerge;
	Microsoft::WRL::ComPtr<IMFSinkWriter> writer;

	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioIn;
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeAudioOut;
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoIn;
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoOut;
	Microsoft::WRL::ComPtr<IMFMediaType> mediaTypeVideoBetween;

	DWORD videoStreamIndexToWrite = -1;
	DWORD audioStreamIndexToWrite = -1;

	bool WriteInner(Microsoft::WRL::ComPtr<IMFSinkWriter> writer, Microsoft::WRL::ComPtr<IMFSourceReader> reader, DWORD readFrom, DWORD writeTo, bool audio);

};