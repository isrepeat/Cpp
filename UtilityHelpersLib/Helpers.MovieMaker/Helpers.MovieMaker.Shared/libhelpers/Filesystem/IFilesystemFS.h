#pragma once
#include "IFolderFS.h"
#include "IStreamFS.h"
#include "ItemResultFS.h"

namespace Filesystem {
	class IFilesystem {
	public:
		IFilesystem();
		virtual ~IFilesystem();

		virtual IFolder *GetInstalledFolder() = 0;
		virtual IFolder *GetLocalFolder() = 0;
		virtual FileResult GetFile(const std::wstring &path) = 0;
		virtual FolderResult GetFolder(const std::wstring &path) = 0;
		virtual IStream *CreateMemoryStream() = 0;

        std::unique_ptr<IFile> GetFileAndCheckToken(const std::wstring &token, std::wstring &resultToken);
        std::unique_ptr<IFolder> GetFolderAndCheckToken(const std::wstring &token, std::wstring &resultToken);

        IFile *CreateFileLocalOrByFullPath(const std::wstring &path, FileCreateMode mode = FileCreateMode::CreateAlways);
        void CreateFileAndWriteStream(const std::wstring& path, IStream* stream);
        IFile *CreateFileInLocalFolder(const std::wstring &name, IStream* fileStream);
	};
}