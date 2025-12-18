#pragma once

#include "App.g.h"

namespace TestVideoPlayer
{
	[Windows::Foundation::Metadata::WebHostHidden]
	ref class App sealed : public Windows::UI::Xaml::Application
	{
	public:
		App();

	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	private:
		void OnNavigationFailed(Platform::Object^ sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs^ e);
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnResuming(Platform::Object^ sender, Platform::Object^ args);
	};
}
