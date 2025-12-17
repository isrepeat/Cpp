#pragma once
#include "IStreamFS.h"
#include "FileAccessModeFS.h"
#include "FileDeleteModeFS.h"

#include <memory>
#include <string>

namespace Filesystem {
	class IFile {
	public:
		IFile();
		virtual ~IFile();

		virtual uint64_t GetSize() const = 0;
		virtual std::wstring GetPath() const = 0;

		virtual IStream *OpenStream(FileAccessMode accessMode) = 0;

		virtual void Delete(FileDeleteMode mode) = 0;

		//virtual void WriteStream(Filesystem::IStream* streamToWrite) = 0;
		virtual bool Exists() const = 0;
	};
}