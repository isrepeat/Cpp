#include "Tools.h"
#include <Helpers/Dx/DxIncludes.h>
#include <Helpers/System.h>
#include <fstream>

namespace DxDesktop {
	namespace Tools {
		std::vector<uint8_t> ReadFile(std::filesystem::path filename) {
			std::vector<uint8_t> buffer;
			buffer.resize(std::filesystem::file_size(filename));

			std::ifstream inFile;
			inFile.open(filename, std::ios::binary);
			inFile.read((char*)buffer.data(), buffer.size());
			inFile.close();

			return buffer;
		}

		std::vector<uint8_t> LoadBGRAImage(std::filesystem::path filename, uint32_t& width, uint32_t& height) {
			HRESULT hr = S_OK;

			Microsoft::WRL::ComPtr<IWICImagingFactory> wicFactory;
			hr = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
			H::System::ThrowIfFailed(hr);

			Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
			hr = wicFactory->CreateDecoderFromFilename(
				filename.wstring().c_str(),
				nullptr,
				GENERIC_READ,
				WICDecodeMetadataCacheOnDemand,
				decoder.GetAddressOf()
			);
			H::System::ThrowIfFailed(hr);

			Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
			hr = decoder->GetFrame(0, frame.GetAddressOf());
			H::System::ThrowIfFailed(hr);

			hr = frame->GetSize(&width, &height);
			H::System::ThrowIfFailed(hr);

			WICPixelFormatGUID pixelFormat;
			hr = frame->GetPixelFormat(&pixelFormat);
			H::System::ThrowIfFailed(hr);

			uint32_t rowPitch = width * sizeof(uint32_t);
			uint32_t imageSize = rowPitch * height;

			std::vector<uint8_t> image;
			image.resize(size_t(imageSize));

			if (memcmp(&pixelFormat, &GUID_WICPixelFormat32bppBGRA, sizeof(GUID)) == 0) {
				hr = frame->CopyPixels(nullptr, rowPitch, imageSize, reinterpret_cast<BYTE*>(image.data()));
				H::System::ThrowIfFailed(hr);
			}
			else {
				Microsoft::WRL::ComPtr<IWICFormatConverter> formatConverter;
				hr = wicFactory->CreateFormatConverter(formatConverter.GetAddressOf());
				H::System::ThrowIfFailed(hr);

				BOOL canConvert = FALSE;
				hr = formatConverter->CanConvert(pixelFormat, GUID_WICPixelFormat32bppBGRA, &canConvert);
				H::System::ThrowIfFailed(hr);
				if (!canConvert) {
					throw std::exception("CanConvert");
				}

				hr = formatConverter->Initialize(
					frame.Get(),
					GUID_WICPixelFormat32bppBGRA,
					WICBitmapDitherTypeErrorDiffusion,
					nullptr,
					0,
					WICBitmapPaletteTypeMedianCut
				);
				H::System::ThrowIfFailed(hr);

				formatConverter->CopyPixels(nullptr, rowPitch, imageSize, reinterpret_cast<BYTE*>(image.data()));
				H::System::ThrowIfFailed(hr);
			}

			return image;
		}
	}
}