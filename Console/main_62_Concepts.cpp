#include <Helpers/FunctionTraits.hpp>
#include <Helpers/Logger.h>
#include <vector>
#include <string>
#include <memory>

namespace TestExtentions {
	namespace concepts {
		template <typename T>
		concept True = true;

		template <typename T>
		concept Trivial = std::is_trivial_v<T>;
	}

	namespace v1 {
		template <typename T>
		struct Base {
			Base() {
				LOG_DEBUG_D("Base()");
			}
		};

		// Specialize 'Base' primary template with concecpt. Cannot inherit from Base<T>.
		// Template overload resoulution preferr this template because it is more strictly than primary.
		template <concepts::True T>
		struct Base<T> {
			Base() {
				LOG_DEBUG_D("[concepts::True] Base()");
			}
		};
	}

	namespace v2 {
		template <typename T>
		struct Base {
			Base() {
				LOG_DEBUG_D("Base()");
			}
		};

		template <typename T>
		struct Wrapper {
			using _Type = T;
		};

		// Specialize 'Base' primary template with <Wrapper<T>> with inheritance from Base<T>.
		template <typename T>
		struct Base<Wrapper<T>> : Base<T> {
			Base() {
				LOG_DEBUG_D("[Wrapper] Base()");
			}
		};
	}

	namespace v3 {
		template <typename T>
		struct Base {
			Base() {
				LOG_DEBUG_D("Base()");
			}
		};

		namespace ex {
			// Use another 'ex::Base' template with that inherited from '::Base<T>'.
			template <typename T>
			struct Base : v3::Base<T> {
				Base() {
					LOG_DEBUG_D("[Ex] Base()");
				}
			};
		}
	}

	void Test() {
		v1::Base<int> instance1{};
		v2::Base<v2::Wrapper<int>> instance2{};
		v3::ex::Base<int> instance3{};
	}
}

namespace TestExistance {
	namespace concepts {
		template<typename T>
		concept HasType = requires {
			typename T;
		};

		template<typename Fn>
		concept HasMethod = 
			H::FunctionTraits<Fn>::Kind == H::FuncKind::ClassMember &&
			H::FunctionTraits<Fn>::IsPointerToMemberFunction == true;

		template<typename Fn, typename Signature>
		concept HasMethodWithSignature =
			H::FunctionTraits<Fn>::Kind == H::FuncKind::ClassMember &&
			H::FunctionTraits<Fn>::IsPointerToMemberFunction == true &&
			std::is_same_v<typename H::FunctionTraits<Fn>::Function, Signature>;

		template<typename Fn>
		concept HasStaticMethod = 
			H::FunctionTraits<Fn>::IsPointerToMemberFunction == false;

		template<typename Fn, typename Signature>
		concept HasStaticMethodWithSignature =
			H::FunctionTraits<Fn>::IsPointerToMemberFunction == false &&
			std::is_same_v<typename H::FunctionTraits<Fn>::Function, Signature>;

		template<bool... _Concepts>
		concept Conjunction = (_Concepts && ...);
	}

	struct Object {
		struct Data {
		};
		int Method(float) {}
		static void StaticMethod() {}
		static int CreateWithDefaultData() { return {}; }
		//static Object CreateWithDefaultData() { return {}; }
	};

	template <typename ObjectT> 
	requires concepts::Conjunction<
		concepts::HasMethodWithSignature<decltype(&ObjectT::Method), int(ObjectT::*)(float)>,
		concepts::HasStaticMethodWithSignature<decltype(&ObjectT::StaticMethod), void(*)()>,
		concepts::HasType<typename ObjectT::Data>
	>
	struct JsonLoader {
		static void Load() {
			ObjectT::StaticMethod();

			if constexpr (requires {
				requires concepts::HasStaticMethodWithSignature<decltype(&ObjectT::CreateWithDefaultData), ObjectT(*)()>;
			}) {
				auto jsonObject = ObjectT::CreateWithDefaultData();
			}
		};
	};

	void Test() {
		JsonLoader<Object>::Load();
	}
}


int main() {
	//TestExtentions::Test();
	TestExistance::Test();
	return 0;
}