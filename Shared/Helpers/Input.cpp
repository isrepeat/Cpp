//#include "pch.h"
#include "Input.h"

namespace H {
	const MousePoint operator*(const MousePoint& point, float factor) {
		return MousePoint{ static_cast<int>(point.x * factor), static_cast<int>(point.y * factor) };
	}

	const MousePoint operator*(float factor, const MousePoint& point) {
		return point * factor;
	}


	int CalculateAbsoluteCoordinateX(int x) {
		return (x * 65536) / GetSystemMetrics(SM_CXSCREEN);
	}

	int CalculateAbsoluteCoordinateY(int y) {
		return (y * 65536) / GetSystemMetrics(SM_CYSCREEN);
	}


	void PerformMouseAction(MouseInput mouseInput) {
		OutputDebugStringA("H::PerformMouseAction(): ");
		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.time = 0;

		switch (mouseInput.action) {
		case MouseAction::Move: {
			MousePoint point = std::get<MousePoint>(mouseInput.data);
			input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
			input.mi.dx = CalculateAbsoluteCoordinateX(point.x);
			input.mi.dy = CalculateAbsoluteCoordinateY(point.y);

			OutputDebugStringA("Move \n");
			break;
		}
		case MouseAction::Press:
		case MouseAction::Release: {
			bool isUp = mouseInput.action == MouseAction::Release;
			MouseButton button = std::get<MouseButton>(mouseInput.data);
			switch (button) {
			case MouseButton::Left:
				input.mi.dwFlags = isUp ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_LEFTDOWN;
				break;
			case MouseButton::Right:
				input.mi.dwFlags = isUp ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_RIGHTDOWN;
				break;
			}

			OutputDebugStringA(isUp ? "Release \n" : "Press \n");

			break;
		}
		}

		SendInput(1, &input, sizeof(input));
	}
}