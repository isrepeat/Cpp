#include <Windows.h>
#include "DiscMaster.h"
#include "DiscRecorder.h"
#include <iostream>
#include <inttypes.h>
#include <string>
#include <vector>

#define CD_RW_1 L"O:"
#define CD_RW_2 L"H:"
#define DefaultBytePerSector 2048

void TestGetDiskSizeViaWin32Api(std::wstring diskRootName = CD_RW_1) {
	ULARGE_INTEGER FreeBytesAvailableToCaller{};
	ULARGE_INTEGER TotalNumberOfBytes{};
	ULARGE_INTEGER TotalNumberOfFreeBytes{};

	GetDiskFreeSpaceExW(diskRootName.c_str(), &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes);
	
	wprintf(L"* GetDiskFreeSpaceEx(\"%s\"): FreeBytesAvailableToCaller = %" PRId64 ", TotalNumberOfBytes = %" PRId64 ", TotalNumberOfFreeBytes = %" PRId64 " \n",
		diskRootName.c_str(), FreeBytesAvailableToCaller.QuadPart, TotalNumberOfBytes.QuadPart, TotalNumberOfFreeBytes.QuadPart);

	return;
}


void TestGetDiskSizeViaIMAPI() {

	return;
}

void TestGetDiskSizeViaIMAPI_2() {
	HRESULT hr = S_OK;

	DiscMaster discMaster;
	auto deviceNumber = discMaster.GetDeviceNumber();

	std::vector<DiscRecorder> discRecroders;
	for (int i = 0; i < deviceNumber; i++) {
		discRecroders.push_back(DiscRecorder{ discMaster.GetUniqueIdForDriveIndex(i) });
	}

	wprintf(L"Get size via win32 api:\n");
	for (auto& discRecorder : discRecroders) {
		TestGetDiskSizeViaWin32Api(discRecorder.GetActiveVolumePath().root_name());
	}

	wprintf(L"\n\n");
	wprintf(L"Get size via IMAPI 2:\n");
	for (auto& discRecorder : discRecroders) {
		auto activeVolumePath = discRecorder.GetActiveVolumePath().root_name().wstring();
		wprintf(L"[Info for \"%s\" ('%c')] \n", activeVolumePath.c_str(), discRecorder.GetActiveVolumeDriveLetter());

		Microsoft::WRL::ComPtr<IDiscFormat2Data> dataWriter;
		hr = CoCreateInstance(__uuidof(MsftDiscFormat2Data), nullptr, CLSCTX_ALL, IID_PPV_ARGS(dataWriter.GetAddressOf()));
		DBREAK_IF_FAIL(hr);

		hr = dataWriter->put_Recorder(discRecorder.GetRecorder().Get());
		DBREAK_IF_FAIL(hr);

		IMAPI_MEDIA_PHYSICAL_TYPE mediaType = IMAPI_MEDIA_TYPE_UNKNOWN;
		hr = dataWriter->get_CurrentPhysicalMediaType(&mediaType);
		DBREAK_IF_FAIL(hr);

		Microsoft::WRL::ComPtr<IFileSystemImage> fileSystemImage;
		hr = CoCreateInstance(__uuidof(MsftFileSystemImage), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&fileSystemImage));
		DBREAK_IF_FAIL(hr);

		hr = fileSystemImage->ChooseImageDefaultsForMediaType(mediaType);
		DBREAK_IF_FAIL(hr);

		VARIANT_BOOL isBlank;
		hr = dataWriter->get_MediaHeuristicallyBlank(&isBlank);
		DBREAK_IF_FAIL(hr);


		FsiFileSystems fileSystems;
		if (isBlank) {
			wprintf(L"* disk is empty \n");
		}
		else {
			wprintf(L"* disk is not empty \n");

			//SAFEARRAYUnique multisessionInterfaces;
			//hr = dataWriter->get_MultisessionInterfaces(GetAddressOf(multisessionInterfaces));
			//DBREAK_IF_FAIL(hr);

			//hr = fileSystemImage->put_MultisessionInterfaces(multisessionInterfaces.get());
			//DBREAK_IF_FAIL(hr);
			//hr = fileSystemImage->ImportFileSystem(&fileSystems);
			//DBREAK_IF_FAIL(hr);
		}

		LONG freeBlocks;
		hr = fileSystemImage->get_FreeMediaBlocks(&freeBlocks);
		DBREAK_IF_FAIL(hr);
		wprintf(L"* freeBlocks = %d \n", freeBlocks);

		LONG usedBlocks;
		hr = fileSystemImage->get_UsedBlocks(&usedBlocks);
		DBREAK_IF_FAIL(hr);
		wprintf(L"* usedBlocks = %d \n", usedBlocks);

		uint64_t freeSpace = (uint64_t)freeBlocks * DefaultBytePerSector;
		wprintf(L"* freeSpace = %" PRId64 " \n", freeSpace);
		
		wprintf(L"\n\n");
	}

	system("pause");
	return;
}


int main(int argc, wchar_t** argv) {
	DBREAK_IF_FAIL(CoInitialize(NULL));

	//if (argc < 2) {
	//	wprintf(L"Usage: \n"
	//		    L"console.exe <Disk>"
	//			L"\n\n"
	//	);
	//	return 0;
	//}



	//TestGetDiskSizeViaWin32Api();
	//TestGetDiskSizeViaIMAPI();
	TestGetDiskSizeViaIMAPI_2();

	CoUninitialize();
	return 0;
}