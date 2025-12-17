#include "pch.h"
#include "IFilesystemFS.h"
#include "..\HText.h"
#include "..\PointerWrappers.h"

#undef CreateFile

namespace Filesystem {
    std::unique_ptr<IFile> IFilesystem::GetFileAndCheckToken(const std::wstring &token, std::wstring &resultToken) {
        auto res = this->GetFile(token);

        if (res.IsTokenNew()) {
            resultToken = res.GetToken();
        }

        return res.DetachItem();
    }

    std::unique_ptr<IFolder> IFilesystem::GetFolderAndCheckToken(const std::wstring &token, std::wstring &resultToken) {
        auto res = this->GetFolder(token);

        if (res.IsTokenNew()) {
            resultToken = res.GetToken();
        }

        return res.DetachItem();
    }

    IFile *IFilesystem::CreateFileLocalOrByFullPath(const std::wstring &path, FileCreateMode mode) {
        bool localPath = path.size() >= 2 && path[1] == ':';
        std::wstring filePath, fileName;
        std::unique_ptr<Filesystem::IFolder> folder;
        auto cleanPath = H::Text::RemoveSlashes(path);

        H::Text::BreakPath(cleanPath, filePath, fileName);

        if (localPath) {
            folder = Unique(this->GetLocalFolder());
        }
        else {
            folder = this->GetFolder(filePath).DetachItem();
        }

        return folder->CreateFile(fileName, mode);
    }

    void IFilesystem::CreateFileAndWriteStream(const std::wstring& path, IStream* stream) {
        auto file = Unique(this->CreateFileLocalOrByFullPath(path, Filesystem::FileCreateMode::CreateAlways));
        auto dstStream = Unique(file->OpenStream(Filesystem::FileAccessMode::ReadWrite));

        const size_t BufSize = 1024;
        uint8_t buffer[BufSize];
        uint32_t written = 0;
        do {
            written = stream->Read(buffer, BufSize);
            dstStream->Write(buffer, written);
        } while (written);
    }

    IFile *IFilesystem::CreateFileInLocalFolder(const std::wstring &name, IStream* fileStream) {
        auto folder = Unique(this->GetLocalFolder());
        auto file = folder->CreateFile(name, Filesystem::FileCreateMode::CreateUnique);
        auto stream = Unique(file->OpenStream(Filesystem::FileAccessMode::ReadWrite));

        const size_t BufSize = 1024;
        uint8_t buffer[BufSize];
        uint32_t written = 0;
        do {
            written = fileStream->Read(buffer, BufSize);
            stream->Write(buffer, written);
        } while (written);

        return file;
    }
}