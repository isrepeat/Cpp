#pragma once
#include <Helpers/common.h>
#include <MagicEnum/MagicEnum.h>
#include <Helpers/FunctionTraits.hpp>
#include <Helpers/Logger.h>
#include <mfapi.h>
#include <wrl.h>

//
// Specialize MF structures for spdlog
//
template<>
struct fmt::formatter<MFRatio, char> {
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.end();
	}
	auto format(const MFRatio& mfRatio, format_context& ctx) -> decltype(ctx.out()) {
		return fmt::format_to(ctx.out(), "{{{}, {}}}", mfRatio.Numerator, mfRatio.Denominator);
	}
};

template<>
struct fmt::formatter<MFRatio, wchar_t> {
	constexpr auto parse(wformat_parse_context& ctx) -> decltype(ctx.begin()) {
		return ctx.end();
	}
	auto format(const MFRatio& mfRatio, wformat_context& ctx) -> decltype(ctx.out()) {
		return fmt::format_to(ctx.out(), L"{{{}, {}}}", mfRatio.Numerator, mfRatio.Denominator);
	}
};


namespace {
	HRESULT _MFGetAttributeRatio(IMFMediaType* pType, const GUID& guidKey, MFRatio& mfRatio) {
		UINT32 nominator;
		UINT32 denominator;
		HRESULT hr = MFGetAttributeRatio(pType, guidKey, &nominator, &denominator);
		if (SUCCEEDED(hr)) {
			mfRatio.Numerator = nominator;
			mfRatio.Denominator = denominator;
		}
		return hr;
	}

#define RETURN_STR_GUID_IF_EQUAL(GUID_VALUE, GUID_KEY)    \
	if (IsEqualGUID(GUID_VALUE, GUID_KEY)) {              \
		return #GUID_KEY;                                 \
	}

	std::string MFGuidToString(const GUID& guid) {
		// Major types
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_Default);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_Audio);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_Video);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_Protected);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_SAMI);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_Script);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_Image);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_HTML);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_Binary);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_FileTransfer);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_Stream);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_MultiplexedFrames);
		RETURN_STR_GUID_IF_EQUAL(guid, MFMediaType_Subtitle);

		// Video subtypes
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_Base);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_RGB32);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_ARGB32);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_RGB24);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_RGB555);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_RGB565);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_RGB8);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_L8);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_L16);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_D16);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_AI44);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_AYUV);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_YUY2);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_YVYU);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_YVU9);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_UYVY);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_NV11);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_NV12);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_YV12);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_I420);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_IYUV);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_Y210);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_Y216);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_Y410);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_Y416);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_Y41P);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_Y41T);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_Y42T);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_P210);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_P216);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_P010);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_P016);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_v210);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_v216);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_v410);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_MP43);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_MP4S);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_M4S2);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_MP4V);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_WMV1);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_WMV2);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_WMV3);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_WVC1);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_MSS1);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_MSS2);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_MPG1);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_DVSL);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_DVSD);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_DVHD);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_DV25);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_DV50);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_DVH1);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_DVC);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_H264);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_H264_ES);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_H265);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_MJPG);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_420O);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_HEVC);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_HEVC_ES);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_VP80);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_VP90);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_ORAW);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_H263);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_A2R10G10B10);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_A16B16G16R16F);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_VP10);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_AV1);
		//RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_Theora);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_MPEG2);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_H264_HDCP);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_HEVC_HDCP);
		RETURN_STR_GUID_IF_EQUAL(guid, MFVideoFormat_Base_HDCP);

		// MPEG-4 media types
		RETURN_STR_GUID_IF_EQUAL(guid, MFMPEG4Format_Base);


		// Audio subtypes
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Base);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_PCM);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Float);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_DTS);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Dolby_AC3_SPDIF);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_DRM);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_WMAudioV8);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_WMAudioV9);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_WMAudio_Lossless);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_WMASPDIF);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_MSP1);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_MP3);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_MPEG);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_AAC);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_AMR_NB);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_AMR_WB);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_AMR_WP);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_FLAC);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_ALAC);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Opus);
		//RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Dolby_AC4);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Dolby_AC3);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Dolby_DDPlus);
		//RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Dolby_AC4_V1);
		//RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Dolby_AC4_V2);
		//RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Dolby_AC4_V1_ES);
		//RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Dolby_AC4_V2_ES);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Vorbis);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_DTS_RAW);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_DTS_HD);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_DTS_XLL);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_DTS_LBR);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_DTS_UHD);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_DTS_UHDY);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_LPCM);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_PCM_HDCP);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Dolby_AC3_HDCP);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_AAC_HDCP);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_ADTS_HDCP);
		RETURN_STR_GUID_IF_EQUAL(guid, MFAudioFormat_Base_HDCP);

		// Image subtypes
		RETURN_STR_GUID_IF_EQUAL(guid, MFImageFormat_JPEG);
		RETURN_STR_GUID_IF_EQUAL(guid, MFImageFormat_RGB32);

		// MPEG subtypes
		RETURN_STR_GUID_IF_EQUAL(guid, MFStreamFormat_MPEG2Transport);
		RETURN_STR_GUID_IF_EQUAL(guid, MFStreamFormat_MPEG2Program);

		return "Unrecognized GUID";
	}
}


#define READ_ATTRIBUTE_TO(GetAttributeFn, pMediaType, ATTRIBUTE_NAME, refResult, ...)                         \
	{                                                                                                         \
		HRESULT hr = GetAttributeFn(pMediaType, ATTRIBUTE_NAME, refResult, ##__VA_ARGS__);   \
		if (FAILED(hr)) {                                                                                     \
			/*LOG_DEBUG_D("- {}: ...", #ATTRIBUTE_NAME);*/                                                    \
		}                                                                                                     \
		else {                                                                                                \
			LOG_DEBUG_D("- {}: {}", #ATTRIBUTE_NAME, refResult);                                              \
		}                                                                                                     \
	}

#define GET_ATTRIBUTE(GetAttributeFn, pMediaType, ATTRIBUTE_NAME, refResult)   \
	{                                                                          \
		auto resultTmp = GetAttributeFn(pMediaType, ATTRIBUTE_NAME, 12345);    \
		if (resultTmp == 12345) {                                              \
			resultTmp = 0;                                                     \
			/*LOG_DEBUG_D("- {}: ...", #ATTRIBUTE_NAME);*/                     \
		}                                                                      \
		else {                                                                 \
			LOG_DEBUG_D("- {}: {}", #ATTRIBUTE_NAME, resultTmp);               \
		}                                                                      \
		refResult = decltype(refResult)(resultTmp);                            \
	}

#define GET_ATTRIBUTE_GUID(pMediaType, GUID_KEY, refResultGUID)                        \
	{                                                                                  \
		HRESULT hr = pMediaType->GetGUID(GUID_KEY, &refResultGUID);                    \
		if (FAILED(hr)) {                                                              \
			/*LOG_DEBUG_D("- {}: ...", #GUID_KEY);*/                                   \
		}                                                                              \
		else {                                                                         \
			LOG_DEBUG_D("- {}: {}", #GUID_KEY, MFGuidToString(refResultGUID));         \
		}                                                                              \
	}

#define GET_ATTRIBUTE_FLAGS(GetAttributeFn, pMediaType, ATTRIBUTE_NAME, result)                      \
	{                                                                                                \
		auto resultTmp = GetAttributeFn(pMediaType, ATTRIBUTE_NAME, 12345);                          \
		if (resultTmp == 12345) {                                                                    \
			resultTmp = 0;                                                                           \
			/*LOG_DEBUG_D("- {}: ...", #ATTRIBUTE_NAME);*/                                           \
		}                                                                                            \
		else {                                                                                       \
			LOG_DEBUG_D("- {}: 0x{:08}", #ATTRIBUTE_NAME, resultTmp);                                \
		}                                                                                            \
		result = decltype(result)(resultTmp);                                                        \
	}

#define GET_ATTRIBUTE_ENUM(GetAttributeFn, pMediaType, ATTRIBUTE_NAME, result)                          \
	{                                                                                                   \
		auto resultTmp = GetAttributeFn(pMediaType, ATTRIBUTE_NAME, 12345);                             \
		if (resultTmp == 12345) {                                                                       \
			resultTmp = 0;                                                                              \
			/*LOG_DEBUG_D("- {}: ...", #ATTRIBUTE_NAME);*/                                              \
		}                                                                                               \
		else {                                                                                          \
			LOG_DEBUG_D("- {}: {}", #ATTRIBUTE_NAME, MagicEnum::ToString(decltype(result)(resultTmp))); \
		}                                                                                               \
		result = decltype(result)(resultTmp);                                                           \
	}


// TODO: add support for getting BLOB data from attributes
//#define READ_BLOB

namespace MEDIA_FOUNDATION_NS {
	namespace Helpers {
		struct GeneralMediaTypeInfo {
			GeneralMediaTypeInfo(IMFMediaType* pMediaType, const std::string& description = {}) {
				LOG_DEBUG_D("GeneralMediaTypeInfo: {}", description);
				GET_ATTRIBUTE_GUID(pMediaType, MF_MT_MAJOR_TYPE, majorType);
				GET_ATTRIBUTE_GUID(pMediaType, MF_MT_SUBTYPE, subType);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pMediaType, MF_MT_ALL_SAMPLES_INDEPENDENT, allSamplesIndependent);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pMediaType, MF_MT_FIXED_SIZE_SAMPLES, fixedSizeSamples);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pMediaType, MF_MT_COMPRESSED, compressed);
				if (fixedSizeSamples) {
					GET_ATTRIBUTE(MFGetAttributeUINT32, pMediaType, MF_MT_SAMPLE_SIZE, sampleSize);
				}
				//GET_ATTRIBUTE(..., pMediaType, MF_MT_AM_FORMAT_TYPE, ...);
				//GET_ATTRIBUTE_BLOB(..., pMediaType, MF_MT_USER_DATA, ...);
				//GET_ATTRIBUTE_BLOB(..., pMediaType, MF_MT_WRAPPED_TYPE, ...);
				LOG_DEBUG_D(" -------------------------------------------");
			}

			GUID majorType;
			GUID subType;
			bool allSamplesIndependent;
			bool fixedSizeSamples;
			bool compressed;
			UINT32 sampleSize;
		};


		struct VideoMediaTypeInfo : GeneralMediaTypeInfo {
			VideoMediaTypeInfo(IMFMediaType* pVideoType, const std::string& description = {}) 
				: GeneralMediaTypeInfo(pVideoType, description)
			{
				LOG_DEBUG_D("VideoMediaTypeInfo: {}", description);
				// VIDEO core data
				READ_ATTRIBUTE_TO(_MFGetAttributeRatio, pVideoType, MF_MT_FRAME_SIZE, frameSize);
				READ_ATTRIBUTE_TO(_MFGetAttributeRatio, pVideoType, MF_MT_FRAME_RATE, frameRate);
				READ_ATTRIBUTE_TO(_MFGetAttributeRatio, pVideoType, MF_MT_PIXEL_ASPECT_RATIO, pixelAspectRatio);

				GET_ATTRIBUTE_FLAGS(MFGetAttributeUINT32, pVideoType, MF_MT_DRM_FLAGS, drmFlags);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_TIMESTAMP_CAN_BE_DTS, timestampCanBeDts);
				GET_ATTRIBUTE_FLAGS(MFGetAttributeUINT32, pVideoType, MF_MT_PAD_CONTROL_FLAGS, videoPadFlags);
				GET_ATTRIBUTE_FLAGS(MFGetAttributeUINT32, pVideoType, MF_MT_SOURCE_CONTENT_HINT, videoSrcContentHintFlags);
				GET_ATTRIBUTE_ENUM(MFGetAttributeUINT32, pVideoType, MF_MT_VIDEO_CHROMA_SITING, videoChromaSubsampling);
				GET_ATTRIBUTE_ENUM(MFGetAttributeUINT32, pVideoType, MF_MT_INTERLACE_MODE, videoInterlaceMode);
				GET_ATTRIBUTE_ENUM(MFGetAttributeUINT32, pVideoType, MF_MT_TRANSFER_FUNCTION, videoTransferFunction);
				GET_ATTRIBUTE_ENUM(MFGetAttributeUINT32, pVideoType, MF_MT_VIDEO_PRIMARIES, videoPrimaries);

				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_MAX_LUMINANCE_LEVEL, maxLuminanceLevel);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_MAX_FRAME_AVERAGE_LUMINANCE_LEVEL, maxFrameAverageLuminanceLevel);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_MAX_MASTERING_LUMINANCE, maxMasteringLuminance);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_MIN_MASTERING_LUMINANCE, minMasteringLuminance);

				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_DECODER_USE_MAX_RESOLUTION, decoderUseMaxResolution);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_DECODER_MAX_DPB_COUNT, decoderMaxDpbCount);

				//READ_ATTRIBUTE_TO(MFGetAttributesAsBlob, pVideoType, MF_MT_CUSTOM_VIDEO_PRIMARIES, &customVideoPrimes, sizeof(customVideoPrimes));
				GET_ATTRIBUTE_ENUM(MFGetAttributeUINT32, pVideoType, MF_MT_DECODER_MAX_DPB_COUNT, videoTransferMatrix);
				GET_ATTRIBUTE_ENUM(MFGetAttributeUINT32, pVideoType, MF_MT_DECODER_MAX_DPB_COUNT, videoLighting);
				GET_ATTRIBUTE_ENUM(MFGetAttributeUINT32, pVideoType, MF_MT_DECODER_MAX_DPB_COUNT, nominalRange);
				//READ_ATTRIBUTE_TO(MFGetAttributesAsBlob, pVideoType, MF_MT_GEOMETRIC_APERTURE, &geometricAperture, sizeof(geometricAperture));
				//READ_ATTRIBUTE_TO(MFGetAttributesAsBlob, pVideoType, MF_MT_MINIMUM_DISPLAY_APERTURE, &minimumDisplayAperture, sizeof(minimumDisplayAperture));
				//READ_ATTRIBUTE_TO(MFGetAttributesAsBlob, pVideoType, MF_MT_PAN_SCAN_APERTURE, &panScanAperture, sizeof(panScanAperture));
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_DECODER_MAX_DPB_COUNT, panScanEnabled);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_DECODER_MAX_DPB_COUNT, averageBitrate);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_DECODER_MAX_DPB_COUNT, averageBitErroRate);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_DECODER_MAX_DPB_COUNT, maxKeyFrameSpacing);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_DECODER_MAX_DPB_COUNT, outputBufferNum);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_DECODER_MAX_DPB_COUNT, realtimeContent);

				// VIDEO - uncompressed format data
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_DEFAULT_STRIDE, defaultStride);
				//LOG_IF_FAILED(MFGetAttributesAsBlob(pVideoType, MF_MT_PALETTE, &pallete, sizeof(pallete)), "failed to get MF_MT_PALETTE");

				// VIDEO - Generic compressed video extra data
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_VIDEO_PROFILE, videoProfile);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pVideoType, MF_MT_VIDEO_LEVEL, videoLevel);

				LOG_DEBUG_D("");
			}

			// VIDEO core data
			MFRatio frameSize;
			MFRatio frameRate;
			MFRatio pixelAspectRatio;
			MFVideoDRMFlags drmFlags;
			bool timestampCanBeDts;
			MFVideoPadFlags videoPadFlags;
			MFVideoSrcContentHintFlags videoSrcContentHintFlags;
			MFVideoChromaSubsampling videoChromaSubsampling;
			MFVideoInterlaceMode videoInterlaceMode;
			MFVideoTransferFunction videoTransferFunction;
			MFVideoPrimaries videoPrimaries;
			UINT32 maxLuminanceLevel;
			UINT32 maxFrameAverageLuminanceLevel;
			UINT32 maxMasteringLuminance;
			UINT32 minMasteringLuminance;
			bool decoderUseMaxResolution;
			UINT32 decoderMaxDpbCount;
#ifdef READ_BLOB
			MT_CUSTOM_VIDEO_PRIMARIES customVideoPrimes;
#endif
			MFVideoTransferMatrix videoTransferMatrix;
			MFVideoLighting videoLighting;
			MFNominalRange nominalRange;
#ifdef READ_BLOB
			MFVideoArea geometricAperture;
			MFVideoArea minimumDisplayAperture;
			MFVideoArea panScanAperture;
#endif
			bool panScanEnabled;
			UINT32 averageBitrate;
			UINT32 averageBitErroRate;
			UINT32 maxKeyFrameSpacing;
			//BLOB mfUserData;
			UINT32 outputBufferNum;
			UINT32 realtimeContent; // 0 or 1

			// VIDEO - uncompressed format data
			UINT32 defaultStride;
#ifdef READ_BLOB
			MFPaletteEntry pallete;
#endif

			// VIDEO - Generic compressed video extra data
			UINT32 videoProfile;
			UINT32 videoLevel;

			// VIDEO - H264 extra data
			// ...
		};


		struct AudioMediaTypeInfo : GeneralMediaTypeInfo {
			AudioMediaTypeInfo(IMFMediaType* pAudioType, const std::string& description = {})
				: GeneralMediaTypeInfo(pAudioType, description)
			{
				LOG_DEBUG_D("AudioMediaTypeInfo: {}", description);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_NUM_CHANNELS, numChannels);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_SAMPLES_PER_SECOND, samplesPerSecond);
				//GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND, floatSamplesPerSecond);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, avgBytesPerSecond);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_BLOCK_ALIGNMENT, blockAlignment);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_VALID_BITS_PER_SAMPLE, validBitsPerSample);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_SAMPLES_PER_BLOCK, samplesPerBlock);
				GET_ATTRIBUTE_FLAGS(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_CHANNEL_MASK, channelMask);
				//GET_ATTRIBUTE_BLOB(..., pAudioType, MF_MT_AUDIO_FOLDDOWN_MATRIX, ...);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_WMADRC_PEAKREF, wmaDrcPeakRef);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_WMADRC_PEAKTARGET, wmaDrcPeakTarget);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_WMADRC_AVGREF, wmaDrcPeakAvgRef);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_WMADRC_AVGTARGET, wmaDrcPeakAvgTarget);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_PREFER_WAVEFORMATEX, prefferWaveFormatEx);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AAC_PAYLOAD_TYPE, aacPayloadType);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION, aacAudioProfileLevelIndication);
				GET_ATTRIBUTE(MFGetAttributeUINT32, pAudioType, MF_MT_AUDIO_FLAC_MAX_BLOCK_SIZE, flacMaxBlockSize);
				LOG_DEBUG_D("");
			}

			UINT32 numChannels;
			UINT32 samplesPerSecond;
			//double floatSamplesPerSecond;
			UINT32 avgBytesPerSecond;
			UINT32 blockAlignment;
			UINT32 bitsPerSample;
			UINT32 validBitsPerSample;
			UINT32 samplesPerBlock;
			UINT32 channelMask;
			//MFFOLDDOWN_MATRIX folddownMatrix;
			UINT32 wmaDrcPeakRef;
			UINT32 wmaDrcPeakTarget;
			UINT32 wmaDrcPeakAvgRef;
			UINT32 wmaDrcPeakAvgTarget;
			bool prefferWaveFormatEx;
			UINT32 aacPayloadType;
			UINT32 aacAudioProfileLevelIndication;
			UINT32 flacMaxBlockSize;
		};


		inline void PrintVideoMediaTypeInfo(Microsoft::WRL::ComPtr<IMFMediaType> videoMediaType, const std::string& description) {
			VideoMediaTypeInfo{ videoMediaType.Get(), description };
		}
		inline void PrintAudioMediaTypeInfo(Microsoft::WRL::ComPtr<IMFMediaType> audioMediaType, const std::string& description) {
			AudioMediaTypeInfo{ audioMediaType.Get(), description };
		}

	} // namespace Helpers
} // namespace MEDIA_FOUNDATION_NS