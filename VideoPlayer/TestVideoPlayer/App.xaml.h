#pragma once

#include "App.g.h"

namespace TestVideoPlayer
{
	ref class App sealed
	{
	public:
		App();
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	private:
		void OnNavigationFailed(Platform::Object^ sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs^ e);
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
	};
}
