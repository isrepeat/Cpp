#pragma once

#include <mfobjects.h>
#include <ObjIdl.h>

namespace Filesystem {
	class IWinStreamExt {
	public:
		IWinStreamExt();
		virtual ~IWinStreamExt();

		virtual void CreateIMFByteStream(IMFByteStream **stream) = 0;
		virtual void CreateIStream(::IStream **stream) = 0;
	};
}