#pragma once

#define NAMESPACE_QLANGUAGE_LIBRARY_START namespace QLanguage { namespace Library {
#define NAMESPACE_QLANGUAGE_LIBRARY_END } }

#include "error.h"
#include "typedef.h"

NAMESPACE_QLANGUAGE_LIBRARY_START
#define THROW_OUT_OF_RANGE throw error<const char*>("out of range", __FILE__, __LINE__)
NAMESPACE_QLANGUAGE_LIBRARY_END

using QLanguage::Library::ushort;
using QLanguage::Library::uint;
using QLanguage::Library::uchar;
using QLanguage::Library::ulong;
using QLanguage::Library::llong;
using QLanguage::Library::ullong;