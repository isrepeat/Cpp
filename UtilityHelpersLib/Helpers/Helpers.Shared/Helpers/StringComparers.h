#pragma once
#include "common.h"
#include <string>

namespace HELPERS_NS {
	struct CaseInsensitiveComparer {
		bool operator()(const std::string& lhs, const std::string& rhs) const {
			return IsLess(lhs, rhs);
		}

		static bool IsLess(const std::string& lhs, const std::string& rhs) {
			return std::lexicographical_compare(
				lhs.begin(), lhs.end(),
				rhs.begin(), rhs.end(),
				[](unsigned char a, unsigned char b) {
					return std::tolower(a) < std::tolower(b);
				}
			);
		}

		static bool IsEqual(const std::string& lhs, const std::string& rhs) {
			if (lhs.size() != rhs.size()) {
				return false;
			}

			for (size_t i = 0; i < lhs.size(); ++i) {
				if (std::tolower(static_cast<unsigned char>(lhs[i])) != std::tolower(static_cast<unsigned char>(rhs[i]))) {
					return false;
				}
			}

			return true;
		}
	};
}