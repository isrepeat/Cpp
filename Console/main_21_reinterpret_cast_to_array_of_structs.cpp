//#include <vector>
//#include <iostream>
//
//
//struct Temp {
//	int a;
//	wchar_t* text;
//	float b;
//};
//
//
//std::vector<Temp> GetTemps() {
//	wchar_t* wstr1 = new wchar_t[] {L"AAA BBB CCC DDD"};
//	wchar_t* wstr2 = new wchar_t[] {L"1111 2222"};
//
//
//	std::vector<Temp> origTemps{ {5, wstr1, 3.14f}, {7, wstr2, 9.11f} };
//	auto ptr1 = reinterpret_cast<uint8_t*>(origTemps.data());
//	auto bytes = std::vector<uint8_t>(ptr1, ptr1 + sizeof(Temp) * origTemps.size());
//
//	//Temp origTemps[2]{ {5, 3.14f}, {7, 9.11f} };
//	//auto ptr1 = reinterpret_cast<uint8_t*>(&(origTemps[0]));
//	//auto bytes = std::vector<uint8_t>(ptr1, ptr1 + sizeof(origTemps));
//	
//	//std::vector<Temp> tmp(2);
//
//	//using aaa = Temp[2];
//	//aaa* tempPtr = reinterpret_cast<aaa*>(bytes.data());
//
//	
//	Temp* tempPtr;
//	tempPtr = reinterpret_cast<Temp*>(bytes.data());
//
//	auto t0 = tempPtr[0];
//	auto t1 = tempPtr[1];
//
//
//	//std::vector<Temp> tempVec;
//	//tempVec.resize(2);
//
//	//auto tempVecPtr = tempVec.data();
//
//	//tempPtr = reinterpret_cast<Temp*>(bytes.data());
//
//	//auto t0 = tempPtr[0];
//	//auto t1 = tempPtr[1];
//
//
//	//using aaa = std::vector<Temp>;
//	//aaa* tempPtr = reinterpret_cast<aaa*>(bytes.data());
//
//	//Temp* tempPtr[2] = reinterpret_cast<Temp*[]>(bytes.data());
//
//	//auto newPtr = tmp.data();
//	//newPtr = origTemps.data();
//
//	////reinterpret_cast<>
//
//
//	std::vector<Temp> newTemp(2);
//	auto ptr2 = reinterpret_cast<uint8_t*>(newTemp.data());
//	std::copy(bytes.begin(), bytes.end(), ptr2);
//
//	return newTemp;
//}
//
//
//int main() {
//	
//	auto temps = GetTemps();
//
//	return 0;
//}