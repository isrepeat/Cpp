#include <Windows.h>
#include <iostream>
#include <memory>

bool IsBadReadPtr_(void* p)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };
	if (::VirtualQuery(p, &mbi, sizeof(mbi)))
	{
		DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
		bool b = !(mbi.Protect & mask);
		// check the page is not a guard page
		if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) b = true;

		return b;
	}
	return true;
}


void Foo(int* p) {
	Sleep(100);
	delete p;
}

int main() {

	std::shared_ptr<int> aaa;

	int* ptr = new int{ 17 };
	bool isBadPtrRead = IsBadReadPtr(ptr, sizeof(decltype(*ptr)));
	bool isBadPtrWrite = IsBadWritePtr(ptr, sizeof(decltype(*ptr)));
	if (isBadPtrRead) {
		Beep(500, 500);
	}
	if (isBadPtrWrite) {
		Beep(500, 500);
	}
	//int* ptr2 = new int{ 17 };
	//int* ptr3 = new int{ 17 };
	Foo(ptr);
	//int* ptr4 = new int{ 17 };
	//int* ptr5 = new int{ 17 };
	//delete ptr;
	bool isBadPtrRead2 = IsBadReadPtr(ptr, sizeof(decltype(*ptr)));
	bool isBadPtrWrite2 = IsBadWritePtr(ptr, sizeof(decltype(*ptr)));
	//bool isValidPtr2_ = IsBadReadPtr_(ptr);
	if (isBadPtrRead2) {
		Beep(1000, 1000);
	}
	if (isBadPtrWrite2) {
		Beep(1000, 1000);
	}

	int val = *ptr;

	return 0;
}
