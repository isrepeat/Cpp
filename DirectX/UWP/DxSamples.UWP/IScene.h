#pragma once

namespace DxSamples {
	class IScene {
	public:
		virtual ~IScene() = default;

		//virtual void Tick() = 0;
		virtual void Render() = 0;
	};
}