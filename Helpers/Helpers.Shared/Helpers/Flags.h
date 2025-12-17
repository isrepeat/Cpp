#pragma once
#include "common.h"
#include <type_traits>
#include <functional>
#include <cassert>

namespace HELPERS_NS {
	// TODO: add other binary operators with flags
	template <typename Enum>
	class Flags {
	public:
		using UnderlyingType = std::underlying_type_t<Enum>; // for most cases it is 'int'

		Flags() = default;
		Flags(int data) : data(data) {}
		Flags(const Enum& enm) : data((UnderlyingType)enm) {}


		Flags operator |= (UnderlyingType mask) {
			this->data |= mask;
			return *this;
		}
		Flags operator &= (UnderlyingType mask) {
			this->data &= mask;
			return *this;
		}
		Flags operator |= (Enum enm) {
			return *this |= (UnderlyingType)enm;
		}
		Flags operator &= (Enum enm) {
			return *this &= (UnderlyingType)enm;
		}

		Flags operator | (UnderlyingType mask) const {
			return Flags(this->data | mask);
		}
		Flags operator & (UnderlyingType mask) const {
			return Flags(this->data & mask);
		}
		Flags operator | (Enum enm) const {
			return *this | (UnderlyingType)enm;
		}
		Flags operator & (Enum enm) const {
			return *this & (UnderlyingType)enm;
		}

		operator UnderlyingType() const {
			return this->data;
		}
		//operator bool() const { // for compare with boolean values (anyway need explicitly cast with boolean compare, so you need override boolean operators ==, != ...)
		//	return (bool)data;
		//}

		// Checks if flags' data have at least one of "mask" bits
		bool Has(UnderlyingType mask) const {
			return this->data & mask;
		}

		// Checks if flags' data have single bit from "enm"
		bool Has(Enum enm) const {
			return this->Has((UnderlyingType)enm);
		}

		// Checks if flags' data have all "mask" bits
		bool HasGroup(UnderlyingType mask) const {
			return (this->data & mask) == mask;
		}

		Enum ToEnum() {
			return static_cast<Enum>(this->data);
		}

		void ProcessAllFlags(std::function<bool(Enum)> handleCallback) const {
			assert(handleCallback);
			if (!handleCallback)
				return;

			UnderlyingType currentBitPos = 1;
			do {
				if (currentBitPos & this->data) {
					if (handleCallback(static_cast<Enum>(currentBitPos))) {
						return;
					}
				}
			} while ((currentBitPos <<= 1) <= this->data);
		}

	private:
		UnderlyingType data = 0;
	};
}