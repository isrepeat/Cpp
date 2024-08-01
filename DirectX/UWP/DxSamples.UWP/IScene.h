#pragma once
#include <Helpers/Math.h>

namespace DxSamples {
	class IScene {
	public:
		virtual ~IScene() = default;

		virtual void Render() = 0;
		virtual void OnWindowSizeChanged(H::Size newSize) = 0;
	};
}