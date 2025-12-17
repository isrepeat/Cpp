#pragma once
#include <MagicEnum/magic_enum/magic_enum.hpp>

namespace MagicEnum {
	template <typename E>
	constexpr auto ToString(E enm) {
		return magic_enum::enum_name(std::forward<E>(enm));
	}
}