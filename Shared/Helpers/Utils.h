#pragma once
#include <memory>

namespace H {
	// https://stackoverflow.com/questions/45507041/how-to-check-if-weak-ptr-is-empty-non-assigned
	template <typename T>
	bool IsWeakPtrUninitialized(const std::weak_ptr<T>& weak) {
		using wt = std::weak_ptr<T>;
		return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
	}

	template <typename T>
	bool IsWeakPtrInitialized(const std::weak_ptr<T>& weak) {
		using wt = std::weak_ptr<T>;;
		return weak.owner_before(wt{}) || wt{}.owner_before(weak);
	}
}
