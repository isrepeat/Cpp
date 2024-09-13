#include <Windows.h>
#include <functional>
#include <algorithm>
#include <iostream>
#include <optional>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <regex>

#include <Helpers/Logger.h>


//#define __LOG_NULLPTR_OBJECT(T) LOG_DEBUG_D("'{}' is nullptr", typeid(T).name())
#define __LOG_NULLPTR_OBJECT(T)

// Extends class with "Try()" method which can be used inside SAFE_RESULT macro
#define SafeObject(T) \
	T* Try() { \
		if (this == nullptr) { \
			__LOG_NULLPTR_OBJECT(T); \
			throw ex::std::bad_pointer(); \
		} \
		return this; \
	}

namespace Helpers {
	namespace ex {
		namespace std {
			struct bad_pointer {};

			template <typename T>
			struct unique_ptr : public ::std::unique_ptr<T> {
				using _MyBase = ::std::unique_ptr<T>;
				using _MyBase::unique_ptr;

				unique_ptr<T>& Try() {
					if (this == nullptr) {
						throw ex::std::bad_pointer{};
					}
					if (this->get() == nullptr) {
						throw ex::std::bad_pointer{};
					}
					return *this;
				}
			};

			template <typename T>
			struct shared_ptr : public ::std::shared_ptr<T> {
				using _MyBase = ::std::shared_ptr<T>;
				using _MyBase::shared_ptr;

				shared_ptr<T>& Try() {
					if (this == nullptr) {
						throw ex::std::bad_pointer{};
					}
					if (this->get() == nullptr) {
						throw ex::std::bad_pointer{};
					}
					return *this;
				}
			};

			template <typename T>
			struct weak_ptr : public ::std::weak_ptr<T> {
				using _MyBase = ::std::weak_ptr<T>;
				using _MyBase::weak_ptr;

				shared_ptr<T> Try() {
					if (this == nullptr) {
						throw ex::std::bad_pointer{};
					}
					auto sharedPtr = this->lock();
					if (sharedPtr == nullptr) {
						throw ex::std::bad_pointer{};
					}
					return sharedPtr;
				}
			};

			// Also you can expose std methods to be available outside through this namespace.
			//using ::std::make_unique;
		}
	}
}

namespace Helpers {
	struct ObjectA {
		SafeObject(ObjectA);

		bool MethodA() {
			return this->condition;
		}

	private:
		bool condition = true;
	};

	struct ObjectB {
		ObjectA* GetObjectA() {
			return nullptr;
			//return std::make_unique<ObjectA>();
		}
	};

	struct ObjectC {
		ex::std::weak_ptr<ObjectB> GetObjectB() {
			//return nullptr;
			return this->objB;
		}

	private:
		std::shared_ptr<ObjectB> objB = std::make_shared<ObjectB>();
	};

	struct ObjectD {
		ex::std::unique_ptr<ObjectC>& GetObjectC() {
			//return nullptr;
			return this->objC;
		}

	private:
		ex::std::unique_ptr<ObjectC> objC = std::make_unique<ObjectC>();
	};

	bool SafeConditionResult(::std::function<bool()> fnCondition) {
		try {
			return static_cast<bool>(fnCondition());
		}
		catch (ex::std::bad_pointer&) {
			return false;
		}
		catch (...) {
			return false;
		}
	}
}

#define SAFE_RESULT(condition) Helpers::SafeConditionResult([&] { return condition;})

int main() {
	Helpers::ObjectD* objectD = new Helpers::ObjectD;

	if (!SAFE_RESULT(objectD->GetObjectC().Try()->GetObjectB().Try()->GetObjectA()->Try()->MethodA())) {
		NOOP;
	}
	return 0;
}


//namespace Standart {
//	template <typename T>
//	struct vector {
//		static constexpr std::string_view templateNotes = "Primary";
//	};
//
//	template <>
//	struct vector<float> {
//		static constexpr std::string_view templateNotes = "<float>";
//	};
//
//	template <typename T>
//	struct pointer {
//		static constexpr std::string_view templateNotes = "Primary";
//		pointer(T*)
//		{}
//		pointer(pointer<T>&& other)
//		{}
//
//	};
//
//	template <>
//	struct pointer<float> {
//		static constexpr std::string_view templateNotes = "<float>";
//	};
//}
//
//namespace Helpers {
//	namespace Ext {
//		namespace Standart {
//			//using ::Standart::vector;
//			template <typename T>
//			struct pointer : public ::Standart::pointer<T> {
//				static constexpr std::string_view templateNotes = "Primary extended";
//				//using _MyBase = ::Standart::pointer<T>;
//				//using ::Standart::pointer<T>::pointer;
//				//pointer(::Standart::pointer<T>&& other) 
//				//	: _MyBase(other)
//				//{
//				//}
//			};
//		}
//	}
//}
//
////namespace Standart {
////	using Helpers::Ext::Standart::pointer;
////}
//
//namespace Helpers {
//	//using namespace ex;
//	//using ex::Standart::pointer;
//	//namespace Standart = ::Standart;
//	
//	//using namespace ::Standart;
//	//using Helpers::Standart::MyTemplate;
//
//
//	//Standart::pointer<int> Foo() {
//	//	return nullptr;
//	//}
//
//	void Do(Standart::pointer<int> ptr) {
//	}
//
//	void InternalTest() {
//		static_assert(Standart::vector<int>::templateNotes == "Primary");
//		static_assert(Standart::vector<float>::templateNotes == "<float>");
//
//		static_assert(Ext::Standart::pointer<int>::templateNotes == "Primary extended");
//		static_assert(Standart::pointer<float>::templateNotes == "<float>");
//
//		//Do(nullptr);
//	}
//}
////namespace Standart = Helpers::Standart;
//
////Standart::pointer<int> Foo() {
////	return nullptr;
////}
//
//void Test() {
//	//Standart::pointer<int>{ nullptr };
//	//Foo();
//	static_assert(Standart::vector<int>::templateNotes == "Primary");
//	static_assert(Standart::vector<float>::templateNotes == "<float>");
//
//	static_assert(Standart::pointer<int>::templateNotes == "Primary extended");
//	static_assert(Standart::pointer<float>::templateNotes == "<float>");
//
//	int* ptr;
//	//Standart::pointer<int>{ptr};
//	//Helpers::Standart::pointer<int>{ptr};
//
//	Helpers::Do(Standart::pointer<int>{ptr});
//}
