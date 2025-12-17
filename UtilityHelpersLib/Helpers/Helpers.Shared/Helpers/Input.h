#pragma once
#include "common.h"
#if COMPILE_FOR_DESKTOP
// TODO: remove this file in release. Use UserInputEvent.h/cpp instead
#include "HWindows.h"
#include <variant>

namespace HELPERS_NS {
	enum class MouseAction {
		Move,
		Press,
		Release,
	};

	enum class MouseButton {
		Left,
		Right,
	};

	struct MousePoint {
		int32_t x = 0;
		int32_t y = 0;
	};

	const MousePoint operator* (const MousePoint& point, float factor);
	const MousePoint operator* (float factor, const MousePoint& point);

	struct MouseInput {
		MouseAction action;
		std::variant<MousePoint, MouseButton> data;
	};


	void PerformMouseAction(MouseInput mouseInput);
}
#endif