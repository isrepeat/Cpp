//#include <atlbase.h>
//#include <initguid.h>
//#include <vds.h>
//#include <string>
//#include <vector>
//#include <memory>
//#include <cstdint>
//#include <comdef.h>
//#include <exception>
//#include <stdexcept>
//#include <array>
//#include <chrono>
//#include <thread>
//
//static void throw_if_fail(HRESULT hr)
//{
//	if (FAILED(hr))
//	{
//		ATL::CComPtr<IErrorInfo> error_info;
//		HRESULT local_hr = GetErrorInfo(0, &error_info);
//		if (local_hr == S_OK)
//			throw _com_error(hr, error_info, true);
//		else
//			throw _com_error(hr);
//	}
//}
//
//
//
//struct VolumeIdentifier {
//	WCHAR			VolumeLetter;
//	GUID			VolumeId;
//	std::wstring	ExistedFileSystem;
//};
//
//
//class VdsDiskFormater {
//public:
//	uint16_t constexpr MakeUDFRevision(uint8_t major, uint8_t minor) {
//		uint16_t revision = (static_cast<uint16_t>(major) << 8) | minor;
//		return revision;
//	}
//
//	const uint16_t DefaultUDFRevision = MakeUDFRevision(2, 1);
//
//	VdsDiskFormater();
//
//	void LoadService();
//	void SetUdfRevision(unsigned short udf_revision);
//	void ReadActiveRecorders();
//
//	void FormatDisk(WCHAR drive_letter, std::wstring label, bool quick_format, float current_progress);
//
//	std::wstring GetExistedFileSystem(WCHAR drive_letter);
//	std::wstring GetExistedFileSystem(_GUID drive_guid);
//	unsigned short GetUDFRevision();
//
//	WCHAR GetVolumeLetter(const std::wstring guid);
//
//private:
//	ATL::CComPtr<IVdsService> m_service;
//	ATL::CComPtr<IVdsServiceLoader> m_loader;
//	unsigned short m_udf_revision;
//	std::wstring m_filesystem_to_create;
//	std::vector<VolumeIdentifier> m_active_recorders;
//
//
//	static ATL::CComPtr<IEnumVdsObject> GetPacks(IUnknown* pVdsSwProviderUnk);
//	static ATL::CComPtr<IEnumVdsObject> GetVolumes(IUnknown* pPackUnk);
//
//	template<class T, class ForEachFn>
//	static void ForEachEnumerable(T* enumerable, ForEachFn forEachFn);
//
//	// void ForEachFn(IVdsVolume2* pVolume, IVdsVolumeMF2* pVolumeMF, const VDS_VOLUME_PROP2& volumeprop, const wchar_t* existedFileSystem, const VDS_FILE_SYSTEM_FORMAT_SUPPORT_PROP* filesystem, LONG& filesystemnum)
//	template<class ForEachFn>
//	static void ForEachVdsVolume(IVdsService* vdsService, ForEachFn forEachFn);
//};
//
//
//#define _SafeRelease(x) {if (NULL != x) { x->Release(); x = NULL; } }
//
//
//VdsDiskFormater::VdsDiskFormater()
//	: m_filesystem_to_create(L"UDF")
//	, m_udf_revision(VdsDiskFormater::DefaultUDFRevision)
//{
//	throw_if_fail(m_loader.CoCreateInstance(CLSID_VdsLoader, NULL, CLSCTX_LOCAL_SERVER));
//}
//
//void VdsDiskFormater::LoadService() {
//	throw_if_fail(m_loader->LoadService(NULL, &m_service));
//	throw_if_fail(m_service->WaitForServiceReady());
//}
//
//void VdsDiskFormater::SetUdfRevision(unsigned short udf_revision) {
//	m_udf_revision = udf_revision;
//}
//
//void VdsDiskFormater::ReadActiveRecorders() {
//	if (!m_active_recorders.empty())
//		m_active_recorders.clear();
//
//	ATL::CComPtr<IEnumVdsObject> pEnumVdsUnallocObject;
//	const size_t LetterCount = ('Z' + 1) - 'A';
//	std::array<VDS_DRIVE_LETTER_PROP, LetterCount> VDS_DRIVE_LETTER_PROP = {};
//
//	throw_if_fail(m_service->QueryDriveLetters(L'A', VDS_DRIVE_LETTER_PROP.size(), VDS_DRIVE_LETTER_PROP.data()));
//
//
//	VolumeIdentifier tmp_identifier;
//	std::vector<VolumeIdentifier> tmp_vector;
//
//	for (auto& i : VDS_DRIVE_LETTER_PROP) {
//		if (i.bUsed == 1) {
//			tmp_identifier.VolumeLetter = i.wcLetter;
//			tmp_identifier.VolumeId = i.volumeId;
//			tmp_vector.push_back(tmp_identifier);
//		}
//	}
//
//	auto& m_active_recorders_ref = m_active_recorders;
//
//	VdsDiskFormater::ForEachVdsVolume(m_service.p,
//		[&tmp_vector, &m_active_recorders_ref](IVdsVolume2* pVolume, IVdsVolumeMF2* pVolumeMF, const VDS_VOLUME_PROP2& volumeprop, const wchar_t* existedFileSystem, const VDS_FILE_SYSTEM_FORMAT_SUPPORT_PROP* filesystem, LONG& filesystemnum)
//		{
//			for (int i = 0; i < tmp_vector.size(); i++)
//			{
//				if (volumeprop.id == tmp_vector.at(i).VolumeId)
//				{
//					std::wstring tmp_uniqueID = std::wstring(volumeprop.pwszName);
//					unsigned int substr = tmp_uniqueID.find(L"\\CdRom");
//
//					if (substr != UINT_MAX)
//					{
//						tmp_vector.at(i).ExistedFileSystem = std::wstring(existedFileSystem);
//						m_active_recorders_ref.push_back(tmp_vector.at(i));
//					}
//				}
//			}
//		});
//}
//
//void VdsDiskFormater::FormatDisk(WCHAR drive_letter, std::wstring label, bool quick_format, float current_progress)
//{
//	if (m_active_recorders.empty())
//		throw std::runtime_error("Error such drive not found.");
//
//	_GUID tmp_guid;
//	bool drive_found = true;
//
//	for (int i = 0; i < m_active_recorders.size(); i++)
//	{
//		if (m_active_recorders.at(i).VolumeLetter == drive_letter)
//		{
//			tmp_guid = m_active_recorders.at(i).VolumeId;
//			drive_found = true;
//			break;
//		}
//		else
//		{
//			drive_found = false;
//		}
//	}
//
//	if (drive_found == false)
//		throw std::runtime_error("Error, such drive not found.");
//
//	auto& m_filesystem_to_create_ref = m_filesystem_to_create;
//	auto& m_udf_revision_ref = m_udf_revision;
//	auto& m_progress_ref = current_progress;
//	//auto& m_cancel_ref = token;
//
//	VdsDiskFormater::ForEachVdsVolume(m_service.p,
//		[&] (IVdsVolume2* pVolume, IVdsVolumeMF2* pVolumeMF, const VDS_VOLUME_PROP2& volumeprop, const wchar_t* existedFileSystem, const VDS_FILE_SYSTEM_FORMAT_SUPPORT_PROP* filesystem, LONG& filesystemnum)
//		{
//			HRESULT hr = S_OK;
//			LPWSTR file_system_type = (LPWSTR)m_filesystem_to_create_ref.c_str();
//			LPWSTR _label = (LPWSTR)label.c_str();
//
//			if (tmp_guid == volumeprop.id)
//			{
//				ATL::CComPtr<IVdsAsync> pAsync;
//
//				//USHORT udfRevision = filesystem[filesystemnum - 1].usRevision;
//				//throw_if_fail(pVolumeMF->FormatEx(file_system_type, udfRevision/*m_udf_revision_ref*/, 0, _label, false, quick_format, false, &pAsync));//ulDesiredUnitAllocationSize  <--- 0 for default
//
//				//hr = pVolumeMF->FormatEx(file_system_type, m_udf_revision_ref, 0, _label, false, quick_format, false, &pAsync);//ulDesiredUnitAllocationSize  <--- 0 for default
//				//hr = pVolumeMF->FormatEx((LPWSTR)L"NTFS", 0, 0, _label, false, quick_format, false, &pAsync);//ulDesiredUnitAllocationSize  <--- 0 for default
//				throw_if_fail(hr);
//
//				if (!pAsync)
//					throw std::runtime_error("Errant provider, returned S_OK, with a NULL pointer.");
//
//				HRESULT hResult = S_OK;
//				VDS_ASYNC_OUTPUT AsyncOut = {};
//
//				ULONG format_percent = 0;
//				while (true)
//				{
//					std::this_thread::sleep_for(std::chrono::seconds(1));
//					HRESULT hrResult = VDS_E_OPERATION_PENDING;
//					throw_if_fail(pAsync->QueryStatus(&hrResult, &format_percent));
//					if (hrResult == VDS_E_BAD_LABEL)
//						break;
//					if (hrResult != VDS_E_OPERATION_PENDING)
//						throw std::runtime_error("Can't get drive formating progress.");
//
//					//m_progress_ref->Report(WriteAction::FormattingMedia, format_percent, MAX_PROGRESS, 0, 0, 0);
//
//					//if (m_cancel_ref->IsCancellationRequested())
//					//{
//					//	throw_if_fail(pAsync->Cancel());
//					//	m_progress_ref->Report(WriteAction::Canceled, 0, MAX_PROGRESS, 0, 0, 0);
//					//	m_cancel_ref->ThrowIfCancellationRequested();
//					//}
//				}
//
//				throw_if_fail(pAsync->Wait(&hResult, &AsyncOut));
//			}
//		});
//}
//
//std::wstring VdsDiskFormater::GetExistedFileSystem(WCHAR drive_letter)
//{
//	if (m_active_recorders.empty())
//		throw std::runtime_error("Active disk recorders not found.");
//
//	for (int i = 0; i < m_active_recorders.size(); i++)
//	{
//		if (m_active_recorders.at(i).VolumeLetter == drive_letter)
//			return m_active_recorders.at(i).ExistedFileSystem;
//	}
//
//	throw std::runtime_error("Active disk recorders not found.");
//}
//
//std::wstring VdsDiskFormater::GetExistedFileSystem(_GUID drive_guid)
//{
//	if (m_active_recorders.empty())
//		throw std::runtime_error("Active disk recorders not found.");
//
//	for (int i = 0; i < m_active_recorders.size(); i++)
//	{
//		if (m_active_recorders.at(i).VolumeId == drive_guid)
//			return m_active_recorders.at(i).ExistedFileSystem;
//	}
//
//	throw std::runtime_error("Active disk recorders not found.");
//}
//
//unsigned short VdsDiskFormater::GetUDFRevision()
//{
//	return m_udf_revision;
//}
//
//WCHAR VdsDiskFormater::GetVolumeLetter(const std::wstring _guid)
//{
//	this->ReadActiveRecorders();
//	OLECHAR* active_guid = nullptr;
//	for (auto device : m_active_recorders)
//	{
//		StringFromCLSID(device.VolumeId, &active_guid);
//		std::wstring tmp_guid = active_guid;
//
//		if (_guid == tmp_guid)
//		{
//			::CoTaskMemFree(active_guid);
//			return device.VolumeLetter;
//		}
//	}
//
//	::CoTaskMemFree(active_guid);
//	throw std::runtime_error("Wrong device ID");
//}
//
//ATL::CComPtr<IEnumVdsObject> VdsDiskFormater::GetPacks(IUnknown* pVdsSwProviderUnk)
//{
//	ATL::CComPtr<IVdsSwProvider> pVdsSwProvider;
//	ATL::CComPtr<IEnumVdsObject> pEnumVdsObjectPack;
//
//	throw_if_fail(pVdsSwProviderUnk->QueryInterface(IID_IVdsSwProvider, (void**)&pVdsSwProvider));
//	throw_if_fail(pVdsSwProvider->QueryPacks(&pEnumVdsObjectPack));
//
//	return pEnumVdsObjectPack;
//}
//
//ATL::CComPtr<IEnumVdsObject> VdsDiskFormater::GetVolumes(IUnknown* pPackUnk)
//{
//	ATL::CComPtr<IVdsPack> pPack;
//	ATL::CComPtr<IEnumVdsObject> pEnumVolumeObject;
//
//	throw_if_fail(pPackUnk->QueryInterface(IID_IVdsPack, (void**)&pPack));
//	throw_if_fail(pPack->QueryVolumes(&pEnumVolumeObject));
//
//	return pEnumVolumeObject;
//}
//
//template<class T, class ForEachFn>
//void VdsDiskFormater::ForEachEnumerable(T* enumerable, ForEachFn forEachFn)
//{
//	while (true) {
//		ULONG ulFetched = 0;
//		ATL::CComPtr<IUnknown> pUnknown;
//
//		auto hResult = enumerable->Next(1, &pUnknown, &ulFetched);
//		if (hResult != S_OK)
//			break;
//
//		forEachFn((IUnknown*)pUnknown);
//	}
//}
//
//template<class ForEachFn>
//void VdsDiskFormater::ForEachVdsVolume(IVdsService* vdsService, ForEachFn forEachFn)
//{
//	ATL::CComPtr<IEnumVdsObject> pEnumVdsSWObject;
//
//	throw_if_fail(vdsService->QueryProviders(VDS_QUERY_SOFTWARE_PROVIDERS, &pEnumVdsSWObject));
//
//	VdsDiskFormater::ForEachEnumerable(pEnumVdsSWObject.p, [&](IUnknown* item) {
//
//		auto pEnumVdsObjectPack = VdsDiskFormater::GetPacks(item);
//		VdsDiskFormater::ForEachEnumerable(pEnumVdsObjectPack.p, [&](IUnknown* itemPack) {
//
//			auto pEnumVolumeObject = VdsDiskFormater::GetVolumes(itemPack);
//			VdsDiskFormater::ForEachEnumerable(pEnumVolumeObject.p, [&](IUnknown* itemVolume) {
//				HRESULT hr = S_OK;
//
//				VDS_VOLUME_PROP2 m_Volumeprop = {};
//				VDS_FILE_SYSTEM_FORMAT_SUPPORT_PROP* m_FileSystemProp = {};
//				LONG fileSystemNum;
//
//				ATL::CComPtr<IVdsVolume2> pVolume;
//				ATL::CComPtr<IVdsVolumeMF2> pVolumeMF;
//				LPWSTR pExistedFileSystem = (LPWSTR)L"Unknown";
//
//				hr = itemVolume->QueryInterface(IID_IVdsVolume2, (void**)&pVolume);
//				throw_if_fail(hr);
//
//				hr = pVolume->GetProperties2(&m_Volumeprop);
//				throw_if_fail(hr);
//
//				hr = itemVolume->QueryInterface(IID_IVdsVolumeMF2, (void**)&pVolumeMF);
//				throw_if_fail(hr);
//
//				hr = pVolumeMF->QueryFileSystemFormatSupport(&m_FileSystemProp, &fileSystemNum);
//				if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_UNRECOGNIZED_MEDIA)) {
//					throw_if_fail(hr);
//				}
//
//				if (SUCCEEDED(hr)) {
//					hr = pVolumeMF->GetFileSystemTypeName(&pExistedFileSystem);
//						if (hr == VDS_E_NO_MEDIA)
//							pExistedFileSystem = (LPWSTR)L"NoMedia";
//						else
//							throw_if_fail(hr);
//				}
//
//				forEachFn(pVolume.p, pVolumeMF.p, m_Volumeprop, pExistedFileSystem, m_FileSystemProp, fileSystemNum);
//				});
//			});
//		});
//}
//
//
//
//int main() {
//	HRESULT hr = CoInitialize(NULL);
//
//	VdsDiskFormater vdsDiskFormatter;
//	vdsDiskFormatter.LoadService();
//	vdsDiskFormatter.ReadActiveRecorders();
//	vdsDiskFormatter.FormatDisk('O', L"MyLabel", true, 0.0f);
//
//	CoUninitialize();
//
//	return 0;
//}