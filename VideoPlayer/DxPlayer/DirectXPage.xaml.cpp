//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"
#include <Helpers/Logger.h>

using namespace DxPlayer;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;

#if ENGINE_TYPE == MEDIA_ENDIGNE_COMPONENT

DirectXPage::DirectXPage()
	: windowVisible(true)
	, coreInput(nullptr)
{
	lg::DefaultLoggers::Init("DxPlayer.log", lg::InitFlags::DefaultFlags | lg::InitFlags::CreateInPackageFolder);

	InitializeComponent();

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	this->playerWinRt = ref new MediaEngineWinRT::Player(this->swapChainPanelXaml);
}

DirectXPage::~DirectXPage()
{
	//// Stop rendering and processing events on destruction.
	//this->main->StopRenderLoop();
	//this->coreInput->Dispatcher->StopProcessEvents();
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
}

// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
}

// DisplayInformation event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
}

// Called when the app bar button is clicked.
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
}

void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
}

#elif ENGINE_TYPE == MEDIA_ENDIGNE_LOGIC

DirectXPage::DirectXPage()
	: windowVisible(true)
	, coreInput(nullptr)
{
	lg::DefaultLoggers::Init("DxPlayer.log", lg::InitFlags::DefaultFlags | lg::InitFlags::CreateInPackageFolder);

	InitializeComponent();

	this->swapChainPanelWinRt = this->CreateSwapChainPanelWinRt(this->swapChainPanelXaml);

	// Register event handlers for page lifecycle.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	this->swapChainPanelXaml->CompositionScaleChanged +=
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	this->swapChainPanelXaml->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	this->swapChainPanelWinRt->SetSwapChainPanelXaml(this->swapChainPanelXaml);

	// Register our SwapChainPanel to get independent input pointer events
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^)
		{
			// The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
			this->coreInput = this->swapChainPanelXaml->CreateCoreIndependentInputSource(
				Windows::UI::Core::CoreInputDeviceTypes::Mouse |
				Windows::UI::Core::CoreInputDeviceTypes::Touch |
				Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

			// Register for pointer events, which will be raised on the background thread.
			this->coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
			this->coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
			this->coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

			// Begin processing input messages as they're delivered.
			this->coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
		});

	// Run task on a dedicated high priority background thread.
	this->inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	this->main = std::unique_ptr<DxPlayerMain>(new DxPlayerMain(this->swapChainPanelWinRt));
	this->main->StartRenderLoop();
}

DirectXPage::~DirectXPage()
{
	// Stop rendering and processing events on destruction.
	this->main->StopRenderLoop();
	this->coreInput->Dispatcher->StopProcessEvents();
}

Helpers::WinRt::Dx::SwapChainPanel^ DirectXPage::CreateSwapChainPanelWinRt(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml) {
	Helpers::WinRt::Dx::SwapChainPanelInitData initData;
	initData.deviceType = Helpers::WinRt::Dx::SwapChainPanelInitData_Device::DxDeviceMF;
	initData.deviceMutexType = Helpers::WinRt::Dx::SwapChainPanelInitData_DeviceMutex::Recursive;
	initData.optionFlags = Helpers::WinRt::Dx::SwapChainPanelInitData_Options::None;

	auto swapChainPanelWinRt = ref new Helpers::WinRt::Dx::SwapChainPanel(initData);
	swapChainPanelWinRt->SetSwapChainPanelXaml(swapChainPanelXaml);
	return swapChainPanelWinRt;
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	this->swapChainPanelWinRt->Trim();

	// Stop rendering when the app is suspended.
	this->main->StopRenderLoop();

	// Put code to save app state here.
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// Put code to load app state here.

	// Start rendering when the app is resumed.
	this->main->StartRenderLoop();
}

// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
}

// DisplayInformation event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	// Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
	// if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
	// you should always retrieve it using the GetDpi method.
	// See DeviceResources.cpp for more details.
	this->swapChainPanelWinRt->SetDpi(sender->LogicalDpi);
	this->main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	this->swapChainPanelWinRt->SetCurrentOrientation(sender->CurrentOrientation);
	this->main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	this->swapChainPanelWinRt->ValidateDevice();
}

// Called when the app bar button is clicked.
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
	// Use the app bar if it is appropriate for your app. Design the app bar, 
	// then fill in event handlers (like this one).
}

void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	this->swapChainPanelWinRt->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	this->main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	this->swapChainPanelWinRt->SetLogicalSize(e->NewSize);
	this->main->CreateWindowSizeDependentResources();
}


#elif ENGINE_TYPE == DX_PLAYER_RENDER

DirectXPage::DirectXPage()
	: windowVisible(true)
	, coreInput(nullptr)
{
	lg::DefaultLoggers::Init("DxPlayer.log", lg::InitFlags::DefaultFlags | lg::InitFlags::CreateInPackageFolder);

	InitializeComponent();

	// Register event handlers for page lifecycle.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	this->swapChainPanelXaml->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	this->swapChainPanelXaml->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	this->deviceResources = std::make_shared<DX::DeviceResources>();
	this->deviceResources->Lock()->SetSwapChainPanel(this->swapChainPanelXaml);

	// Register our SwapChainPanel to get independent input pointer events
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
		this->coreInput = this->swapChainPanelXaml->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		// Register for pointer events, which will be raised on the background thread.
		this->coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
		this->coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
		this->coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

		// Begin processing input messages as they're delivered.
		this->coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// Run task on a dedicated high priority background thread.
	this->inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

	this->main = std::unique_ptr<DxPlayerMain>(new DxPlayerMain(this->deviceResources));
	this->main->StartRenderLoop();
}

DirectXPage::~DirectXPage()
{
	// Stop rendering and processing events on destruction.
	this->main->StopRenderLoop();
	this->coreInput->Dispatcher->StopProcessEvents();
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	this->deviceResources->Lock()->Trim();

	// Stop rendering when the app is suspended.
	this->main->StopRenderLoop();

	// Put code to save app state here.
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// Put code to load app state here.

	// Start rendering when the app is resumed.
	this->main->StartRenderLoop();
}

// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
}

// DisplayInformation event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	// Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
	// if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
	// you should always retrieve it using the GetDpi method.
	// See DeviceResources.cpp for more details.
	this->deviceResources->Lock()->SetDpi(sender->LogicalDpi);
	this->main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	this->deviceResources->Lock()->SetCurrentOrientation(sender->CurrentOrientation);
	this->main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	this->deviceResources->Lock()->ValidateDevice();
}

// Called when the app bar button is clicked.
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
	// Use the app bar if it is appropriate for your app. Design the app bar, 
	// then fill in event handlers (like this one).
}

void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
	// When the pointer is pressed begin tracking the pointer movement.
	this->main->StartTracking();
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
	// Update the pointer tracking code.
	if (this->main->IsTracking())
	{
		this->main->TrackingUpdate(e->CurrentPoint->Position.X);
	}
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
	// Stop tracking pointer movement when the pointer is released.
	this->main->StopTracking();
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	this->deviceResources->Lock()->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	this->main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(this->main->GetCriticalSection());
	this->deviceResources->Lock()->SetLogicalSize(e->NewSize);
	this->main->CreateWindowSizeDependentResources();
}
#endif