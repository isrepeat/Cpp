#include "PropertyStoreReader.h"
#include "SmartPROPVARIANT.h"
#include "System.h"

using namespace HELPERS_NS;

PropertyStoreReader::PropertyStoreReader(IPropertyStore* propStore)
    : propStore(propStore)
{}

std::wstring PropertyStoreReader::ReadWStringHelper(const PROPERTYKEY& propKey) const {
    HRESULT hr = S_OK;
    SmartPROPVARIANT propVar;

    hr = this->propStore->GetValue(propKey, propVar.get());
    H::System::ThrowIfFailed(hr);

    if (propVar->vt == VT_LPWSTR && propVar->pwszVal) {
        return propVar->pwszVal;
    }

    return {};
}
