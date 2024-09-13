#include <Windows.h>
#include <functional>
#include <algorithm>
#include <iostream>
#include <optional>
#include <fstream>
#include <vector>
#include <string>
#include <regex>

#include <Helpers/Logger.h>
#include <Helpers/FunctionTraits.hpp>

template <typename T>
struct Object {
	virtual ~Object() = default;

	T* operator->() {
		LOG_DEBUG_D("[{}]", typeid(T).name());
		return dynamic_cast<T*>(this);
	}
};

#define SAFE_OBJ(T) Object<T>

template <typename T>
struct PtrWrapper {
	PtrWrapper() 
		: ptr{ nullptr }
	{}
	PtrWrapper(T* ptr)
		: ptr{ ptr }
	{}
	
	T& operator->() {
		return *this->ptr;
	}

private:
	T* ptr;
};

struct ObjectA : SAFE_OBJ(ObjectA) {
	bool MethodA() {
		return this->condition;
	}

private:
	bool condition = true;
};

struct ObjectB : SAFE_OBJ(ObjectB) {
	PtrWrapper<ObjectA> GetObjectA() {
		return nullptr;
	}
};

struct ObjectC : SAFE_OBJ(ObjectC) {
	ObjectB* GetObjectB() {
		return nullptr;
	}
};

struct ObjectD : SAFE_OBJ(ObjectD) {
	PtrWrapper<ObjectC> GetObjectC() {
		return nullptr;
	}
};






//template <typename T>
//struct Wrapper {
//	Wrapper(T* ptr)
//		: ptr{ ptr }
//	{}
//	
//	T* operator->() {
//		return this->ptr;
//	}
//
//	void Hello();
//
//private:
//	T* ptr;
//};

//template <typename T>
//struct WrapperProxy;

template <typename T>
struct WrapperBase {
	WrapperBase(T* ptr)
		: ptr{ ptr }
	{}

	//T* operator->() {
	//	return this->ptr;
	//}

	WrapperBase* operator->() {
		return this;
	}

	//template<typename U>
	//void operator()() {
	//}

	T* get() {
		return this->ptr;
	}

private:
	T* ptr;
};

template <typename T>
struct WrapperProxy {
	WrapperProxy(T* ptr)
		: wrapperBase{ ptr }
	{}

	WrapperBase<T>& operator->() {
		return this->wrapperBase;
	}

private:
	WrapperBase<T> wrapperBase;
};

template <typename T>
struct safe_ptr : std::unique_ptr<T> {
	safe_ptr(T* ptr)
		: std::unique_ptr<T>(ptr ? ptr : &this->defaultObj)
	{}

	virtual ~safe_ptr() {
		if (::std::unique_ptr<T>::get() == &this->defaultObj) {
			::std::unique_ptr<T>::release();
		}
	}

	//using std::unique_ptr<T>::operator*;
	//using std::unique_ptr<T>::operator->;

private:
	T defaultObj;
};


//struct Temp {};
//
//template <typename... Args>
////Wrapper<ResultT> operator>> (Temp value, ResultT* result) { // extract a signed char
//void operator>> (Temp value, Args... result) { // extract a signed char
//	//return result;
//}
//
//////template <typename ResultT>
////ObjectB operator>> (ObjectC value, ObjectB result) { // extract a signed char
////	return {};
////}


// Our example class on which we'll be calling our member function pointer (MFP)
struct AAA {
	int aaa() {
		return 1337;
	}
};

struct BBB {
	AAA* bbb() {
		return nullptr;
	}
};

template <typename T> 
safe_ptr<T> SafeCall(T* result) {
	return safe_ptr<T>{result};
}

int main() {
//	safe_ptr<ObjectD> safeObjectD(nullptr);

	//constexpr auto xxx = typeid(ObjectA).before(;
	ObjectD objectD;
	objectD->GetObjectC()->GetObjectB()->GetObjectA()->MethodA();

	//safeObjectD->GetObjectC();
	//safe_ptr<ObjectC>{safeObjectD->GetObjectC()}->GetObjectB();
	//safe_ptr<ObjectB>{safe_ptr<ObjectC>{safeObjectD->GetObjectC()}->GetObjectB()}->GetObjectA();

	//SafeCall(safeObjectD->GetObjectC())->GetObjectB();

	//auto ptrAAA = new AAA;
	//auto ptrBBB = new BBB;
	//
	////BBB sss;
	////decltype(sss);

	//((ptrBBB->*BBB::bbb)()->*AAA::aaa)();

	////ObjectD objectD;
	//////objectD->


	//////WrapperBase<ObjectD> wrapperBase{ &objectD };
	//////wrapperBase<int>()

	////WrapperProxy<ObjectD> wrapperProxy{ &objectD };
	////wrapperProxy->

	//////Temp{} >> 1, 2.0f, 3.0;
	//////((Temp{} >> objectD->GetObjectC()).GetObjectB())

	//////(12 >> objectD.GetObjectC()).;
	return 0;
}


//
//// Safe<ObjectB> safeB
//// if (ToFalseIfSomeObjInChainIsNull(safeB->GetObjectA()->MethodA())) {
////											returned Safe<ObjectA>
////     ...
//// }
//// 
//// if (ToFalseIfSomeObjInChainIsNull(Safe<ObjectA>->MethodA())) {
////     ...
//// }
//// Если цепочка
//
//struct NullObjException {};
//
//
////template <typename CurrentT, typename NextT>
////struct ResultWrapper : public CurrentT {
////	ResultWrapper<NextT, int> operator->() {
////		return this->next;
////	}
////
////private:
////	ResultWrapper<NextT, int>* next;
////};
//
//
//template <typename CurrentT, typename NextT>
//struct ResultWrapper : public CurrentT {
//	//std::unique_ptr<ResultWrapper<ObjectA>> operator->() {
//	//	return this->next;
//	//}
//
//	ResultWrapper<NextT, void> operator->() {
//		return this->obj;
//	}
//
//private:
//	//ResultWrapper<NextT, void> obj;
//	//std::unique_ptr<ResultWrapper<ObjectA>> next;
//};
//
//template <typename CurrentT>
//struct ResultWrapper<CurrentT, void> : CurrentT {
//
//	CurrentT* operator->() {
//		return this;
//	}
//};
//
//
//int main() {
//	//std::unique_ptr<ResultWrapper<ObjectB>> wrapperB;
//	//wrapperB->operator->()->MethodA();
//
//	ResultWrapper<ObjectC, ObjectB> wrapperB;
//	wrapperB->
//
//	return 0;
//}
//


//struct client
//{
//	int a;
//};
//
//struct proxy {
//	client* target;
//	client* operator->() const
//	{
//		return target;
//	}
//};
//
//struct proxy2 {
//	proxy* target;
//	proxy& operator->() const
//	{
//		return *target;
//	}
//};
//
//void f() {
//	client x;
//	//std::unique_ptr<proxy> y;
//	proxy y;
//	proxy2 z;
//
//	//std::cout << x.a << y->operator->()->a << z->a; // print "333"
//	std::cout << x.a << y->a << z->a; // print "333"
//}


//




//template <typename T>
//struct SafeWrapper {
//
//	SafeWrapper(T* objPtr)
//		: objPtr{ objPtr }
//	{}
//
//	//template <typename... FnArgs, typename... RealArgs>
//	//bool Invoke(bool(T::*memberMethod)(FnArgs...), RealArgs&&... args) {
//	//	if (this->objPtr) {
//	//		return std::invoke(memberMethod, this->objPtr, std::forward<RealArgs&&>(args)...);
//	//	}
//	//	return false;
//	//}
//
//	template <typename Fn, typename... RealArgs, typename _Ret = H::FunctionTraits<Fn>::Ret>
//	SafeWrapper<_Ret> Invoke(Fn func, RealArgs&&... args) {
//		static_assert(H::FunctionTraits<Fn>::Kind == H::FuncKind::ClassMember, "func 'Fn' is not classMember");
//		static_assert(std::is_same_v<H::FunctionTraits<Fn>::Class, T>, "func 'Fn' is not belong to the 'T'");
//
//		if (this->objPtr) {
//			return SafeWrapper<_Ret>{std::invoke(func, this->objPtr, std::forward<RealArgs&&>(args)...)};
//		}
//		return SafeWrapper<_Ret>{};
//	}
//
//	operator bool() const {
//		if (this->objPtr) {
//		}
//		else {
//			return false;
//		}
//	}
//
//private:
//	T* objPtr;
//};

//template <typename T>
//struct SafeWrapper2 {
//	SafeWrapper2(T* objPtr)
//		: objPtr{ objPtr }
//	{}
//
//	T* operator->() {
//		if (this->objPtr) {
//			return this->objPtr;
//		}
//		else {
//			return nullptr;
//		}
//	}
//
//private:
//	T* objPtr;
//};




//int main() {
//
//	//ObjectA* objectA = new ObjectA{};
//	ObjectA* objectA = nullptr;
//	ObjectB* objectB = nullptr;
//	
//	//auto safeWrapperA = SafeWrapper<ObjectA>(objectA);
//	//if (safeWrapperA.Invoke(&ObjectA::MethodA)) {
//	//	LOG_DEBUG_D("safeWrapperA ==> true");
//	//}
//
//	auto safeWrapperB = SafeWrapper<ObjectB>(objectB);
//	if (safeWrapperB.Invoke(&ObjectB::GetObjectA)) {
//		LOG_DEBUG_D("safeWrapperB ==> true");
//	}
//
//	//auto safeWrapper2 = SafeWrapper2<ObjectA>(objectA);
//	//if (safeWrapper2->MethodA()) {
//	//	LOG_DEBUG_D("safeWrapper2 ==> true");
//	//}
//	return 0;
//}