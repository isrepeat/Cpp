#include <Windows.h>
#include <iostream>
#include <string>

int main() {

	GUID guid = { 0x09329B74, 0x40A3, 0x4C68, { 0xBF, 0x07,  0xAF,  0x9A,  0x57,  0x2F,  0x60,  0x7C } };

	std::wstring strGuid(64, '\0');
	StringFromGUID2(guid, strGuid.data(), strGuid.size());

	return 0;
}
