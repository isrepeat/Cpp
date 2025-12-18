#include "pch.h"
#include "App.xaml.h"
#include "MainPage.xaml.h"

using namespace TestVideoPlayer;
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
		DebugSettings->EnableFrameRateCounter = false;
	}
#endif

	auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

	if (rootFrame == nullptr)
	{
		rootFrame = ref new Frame();
		rootFrame->NavigationFailed += ref new NavigationFailedEventHandler(this, &App::OnNavigationFailed);
		Window::Current->Content = rootFrame;
	}

	if (rootFrame->Content == nullptr)
	{
		rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
	}

	Window::Current->Activate();
}

void App::OnNavigationFailed(Object^ sender, NavigationFailedEventArgs^ e)
{
	throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}

void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
	(void)sender;
	(void)e;
}
