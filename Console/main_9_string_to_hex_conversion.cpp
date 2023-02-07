//#include <Windows.h>
//#include <iostream>
//#include <sstream>
//#include <string>
//#include <iomanip>
//
//template<typename T>
//std::string HexToString(T hexValue) {
//	std::stringstream ss;
//	ss << "0x" << std::hex << std::setw(sizeof(hexValue) * 2) << std::setfill('0') << hexValue;
//	return ss.str();
//}
//
//
//template<typename T>
//T HexFromString(const std::string& hexString) {
//	T hexValue;
//	std::stringstream ss;
//	ss << std::hex << hexString;
//	ss >> hexValue;
//	return hexValue;
//}
//
//int main() {
//	
//	//int hexValue = 0xff;
//	
//	auto hexString = HexToString(0x00ff);
//	auto hexValue = HexFromString<int>(hexString);
//
//
//	//std::string hexString;
//	//{
//	//	std::stringstream ss;
//	//	ss << std::hex << std::setw(sizeof(hexValue) * 2) << std::setfill('0') << hexValue;
//	//	hexString = ss.str();
//	//}
//
//
//	//std::string s = "0x00ff";
//	//int x = std::stoul(s, nullptr, 16);
//
//	//int newHexValue;
//	//{
//	//	std::stringstream ss;
//	//	ss << std::hex << "0x00ff";
//	//	ss >> newHexValue;
//	//}
//	return 0;
//}