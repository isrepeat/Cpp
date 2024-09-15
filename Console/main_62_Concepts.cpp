#include <Helpers/Logger.h>
#include <vector>
#include <string>
#include <memory>

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


int main() {
	v1::Base<int> instance1{};
	v2::Base<v2::Wrapper<int>> instance2{};
	v3::ex::Base<int> instance3{};
	return 0;
}