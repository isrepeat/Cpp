#pragma once
#include "config.h"

#include <functional>
#include <memory>
#include <DirectXMath.h>

#if HAVE_WINRT == 1

ref class GestureHelper sealed {
public:
	GestureHelper();

	void ProcessPress(Windows::UI::Input::PointerPoint ^ppt);
	void ProcessMove(Windows::UI::Input::PointerPoint ^ppt);
	void ProcessRelease(Windows::UI::Input::PointerPoint ^ppt);

internal:
	std::function<void(float x, float y, Windows::Devices::Input::PointerDeviceType pointerType)> ManipulationStarted;
	std::function<void(const DirectX::XMFLOAT2 &movVec, const DirectX::XMFLOAT2 &newPos)> MoveUpdated;
	std::function<void(const DirectX::XMFLOAT2 &pos)> ManipulationCompleted;
	std::function<void(float scale, float x, float y)> ZoomUpdated;
	std::function<void(float angle, float x, float y)> RotateUpdated;
	std::function<void(int tapCount, float x, float y)> Tapped;

private:
	Windows::UI::Input::GestureRecognizer ^recognizer;

	// Recognizer handling functions.
	void OnManipulationStarted(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationStartedEventArgs^ e);
	void OnManipulationCompleted(Windows::UI::Input::GestureRecognizer ^sender, Windows::UI::Input::ManipulationCompletedEventArgs ^e);
	void OnManipulationUpdated(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationUpdatedEventArgs^ e);
	void OnTapped(Windows::UI::Input::GestureRecognizer ^sender, Windows::UI::Input::TappedEventArgs ^e);
};

#endif // HAVE_WINRT