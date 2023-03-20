//#include <Windows.h>
//#include <iostream>
//
//
//void Foo(int* p) {
//	Sleep(100);
//	delete p;
//}
//
//int main() {
//
//	int* ptr = new int{ 17 };
//	bool isValidPtr = IsBadReadPtr(ptr, sizeof(decltype(*ptr)));
//	if (isValidPtr) {
//		Beep(500, 500);
//	}
//	//int* ptr2 = new int{ 17 };
//	//int* ptr3 = new int{ 17 };
//	Foo(ptr);
//	//int* ptr4 = new int{ 17 };
//	//int* ptr5 = new int{ 17 };
//	//delete ptr;
//	bool isValidPtr2 = IsBadReadPtr(ptr, sizeof(decltype(*ptr)));
//	if (isValidPtr2) {
//		Beep(1000, 1000);
//	}
//
//	int val = *ptr;
//
//	return 0;
//}
