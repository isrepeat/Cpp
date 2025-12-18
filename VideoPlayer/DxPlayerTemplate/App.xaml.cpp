#include "pch.h"
#include "App.xaml.h"
#include "DirectXPage.xaml.h"

using namespace DxPlayerTemplate;

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

App::App()
{
	InitializeComponent();
	Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
}

void App::OnLaunched(LaunchActivatedEventArgs^ e)
{
#if _DEBUG
	if (IsDebuggerPresent())
	{
		DebugSettings->EnableFrameRateCounter = true;
	}
#endif

	auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

	// Do not repeat app initialization when the Window already has content,
	// just ensure that the window is active
	if (rootFrame == nullptr)
	{
		rootFrame = ref new Frame();
		rootFrame->NavigationFailed += ref new NavigationFailedEventHandler(this, &App::OnNavigationFailed);

		Window::Current->Content = rootFrame;
	}

	if (rootFrame->Content == nullptr)
	{
		rootFrame->Navigate(Windows::UI::Xaml::Interop::TypeName(DirectXPage::typeid), e->Arguments);
	}

	Window::Current->Activate();
}

void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
	(void)sender;
	auto deferral = e->SuspendingOperation->GetDeferral();
	// TODO: Save application state here.
	deferral->Complete();
}

void App::OnNavigationFailed(Object ^sender, NavigationFailedEventArgs ^e)
{
	(void)sender;
	throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}
