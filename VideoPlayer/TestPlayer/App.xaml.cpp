//
// App.xaml.cpp
// Implementation of the App class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace TestPlayer;

using namespace Platform;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

/// <summary>
/// Initializes the singleton application object.
/// </summary>
App::App()
{
	InitializeComponent();
}

/// <summary>
/// Invoked when the application is launched normally by the end user.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(LaunchActivatedEventArgs^ e)
{
	auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

	if (rootFrame == nullptr)
	{
		rootFrame = ref new Frame();
		rootFrame->NavigationFailed += ref new NavigationFailedEventHandler(this, &App::OnNavigationFailed);
		Window::Current->Content = rootFrame;
	}

	if (rootFrame->Content == nullptr)
	{
		rootFrame->Navigate(DirectXPage::typeid, e->Arguments);
	}

	Window::Current->Activate();
}

/// <summary>
/// Invoked when Navigation to a certain page fails.
/// </summary>
/// <param name="sender">The Frame which failed navigation.</param>
/// <param name="e">Details about the navigation failure.</param>
void App::OnNavigationFailed(Object ^sender, NavigationFailedEventArgs ^e)
{
	throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}
