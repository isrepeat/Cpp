//#include <Windows.h>
//#include <iostream>
//#include <vector>
//#include <span>
//
//// Convert some array's byte to hex
//std::span<char>::iterator HexByte(BYTE byte, std::span<char>::iterator arrIt, const std::span<char>::iterator arrItEnd) {
//	static const char* digits{ "0123456789ABCDEF" }; // NOTE: byte must be unsigned value
//	
//	if (arrIt < arrItEnd - 1) {
//		*arrIt++ = digits[byte >> 4];
//		*arrIt++ = digits[byte & 0x0f];
//	}
//	return arrIt;
//}
//
//int main() {
//	char bytes[16] = {
//		0x01, 0x02, 0x03, 0x04, 0x0A, 0x0B, 0x0C, 0x0D,
//		0x10, 0x20, 0x30, 0x40, 0xA0, 0xB0, 0xC0, 0xD0 
//	};
//
//	//char uuidStr[36];
//	std::string uuidStr(36, '\0');
//	std::span<char> spanUuid = uuidStr;
//	auto it = spanUuid.begin();
//	const auto itEnd = spanUuid.end();
//
//	it = HexByte(bytes[0], it, itEnd);
//	it = HexByte(bytes[1], it, itEnd);
//	it = HexByte(bytes[2], it, itEnd);
//	it = HexByte(bytes[3], it, itEnd);
//	*it++ = '-';
//	it = HexByte(bytes[4], it, itEnd);
//	it = HexByte(bytes[5], it, itEnd);
//	*it++ = '-';
//	it = HexByte(bytes[6], it, itEnd);
//	it = HexByte(bytes[7], it, itEnd);
//	*it++ = '-';
//	it = HexByte(bytes[8], it, itEnd);
//	it = HexByte(bytes[9], it, itEnd);
//	it = HexByte(bytes[10], it, itEnd);
//	it = HexByte(bytes[11], it, itEnd);
//	*it++ = '-';
//	it = HexByte(bytes[12], it, itEnd);
//	it = HexByte(bytes[13], it, itEnd);
//	it = HexByte(bytes[14], it, itEnd);
//	it = HexByte(bytes[15], it, itEnd);
//	
//	int xxx = 9;
//
//	return 0;
//}