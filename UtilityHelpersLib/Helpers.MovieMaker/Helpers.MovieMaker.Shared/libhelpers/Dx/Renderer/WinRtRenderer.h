#pragma once
#include "SwapChainPanelOutput.h"
#include "../Dx.h"
#include "../../Thread/critical_section.h"
#include "../../HSystem.h"
#include "../../HMath.h"
#include "../../WinRT/RunOnUIThread.h"

#include <memory>
#include <thread>
#include <type_traits>

enum class RenderThreadState {
    Work,
    Pause, // can be changed to Work to let thread continue and not exit
    Stop, // thread will exit very soon
};

#ifdef WINRT_Any
template<class T>
class WinRtRenderer {
public:
    template<class... Args>
    WinRtRenderer(Helpers::WinRt::Dx::SwapChainPanel^ swapChainPanelWinRt, Args&&... args)
        : swapChainPanelWinRt{ swapChainPanelWinRt }
        , swapChainPanelXaml{ swapChainPanelWinRt->GetSwapChainPanelXaml() }
        , swapChainPanelOutput{ this->swapChainPanelWinRt }
        , renderer(this->swapChainPanelOutput.GetSwapChainPanelNative()->GetDxDevice(), &this->swapChainPanelOutput, std::forward<Args>(args)...)
        , pointerMoves(false)
        , renderThreadState(RenderThreadState::Stop)
    {
        H::RunOnUIThread(this->swapChainPanelXaml->Dispatcher,
            [this]
            {
                auto window = Windows::UI::Xaml::Window::Current->CoreWindow;
                auto displayInformation = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();

                this->wnd = Windows::UI::Xaml::Window::Current;

                this->visChangedToken = window->VisibilityChanged += H::System::MakeTypedEventHandler(
                    [=](Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args)
                    {
                        if (!args->Visible) {
                            this->renderThreadState = RenderThreadState::Pause;
                        }
                        else {
                            if (this->renderThreadState == RenderThreadState::Stop) {
                                if (this->renderThread.joinable()) {
                                    this->renderThread.join();
                                }

                                this->StartRenderThread();
                            }
                            else {
                                this->renderThreadState = RenderThreadState::Work;
                            }
                        }
                    });

                this->orientChangedToken = displayInformation->OrientationChanged += H::System::MakeTypedEventHandler(
                    [=](Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args)
                    {
                        LOG_FUNCTION_ENTER("displayInformation->OrientationChanged()");
                        concurrency::critical_section::scoped_lock lk(this->cs);
                        auto orientation = sender->CurrentOrientation;
                        auto dpi = sender->LogicalDpi;
                        auto scale = DirectX::XMFLOAT2(this->swapChainPanelXaml->CompositionScaleX, this->swapChainPanelXaml->CompositionScaleY);
                        auto size = DirectX::XMFLOAT2((float)this->swapChainPanelXaml->ActualWidth, (float)this->swapChainPanelXaml->ActualHeight);

                        this->TryResize(size, scale, dpi, orientation);
                    });

                this->dispContentInvalidatedToken = displayInformation->DisplayContentsInvalidated += H::System::MakeTypedEventHandler(
                    [=](Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args)
                    {
                        LOG_FUNCTION_ENTER("displayInformation->DisplayContentsInvalidated()");
                    });

                this->dpiChangedToken = displayInformation->DpiChanged += H::System::MakeTypedEventHandler(
                    [=](Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args)
                    {
                        LOG_FUNCTION_ENTER("displayInformation->DpiChanged()");
                        concurrency::critical_section::scoped_lock lk(this->cs);
                        auto orientation = sender->CurrentOrientation;
                        auto dpi = sender->LogicalDpi;
                        auto scale = DirectX::XMFLOAT2(this->swapChainPanelXaml->CompositionScaleX, this->swapChainPanelXaml->CompositionScaleY);
                        auto size = DirectX::XMFLOAT2((float)this->swapChainPanelXaml->ActualWidth, (float)this->swapChainPanelXaml->ActualHeight);

                        this->TryResize(size, scale, dpi, orientation);
                    });

                this->compScaleChangedToken = this->swapChainPanelXaml->CompositionScaleChanged += H::System::MakeTypedEventHandler(
                    [=](Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Platform::Object^ args)
                    {
                        LOG_FUNCTION_ENTER("displayInformation->CompositionScaleChanged()");
                        auto displayInformation = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();

                        concurrency::critical_section::scoped_lock lk(this->cs);
                        auto orientation = displayInformation->CurrentOrientation;
                        auto dpi = displayInformation->LogicalDpi;
                        auto scale = DirectX::XMFLOAT2(sender->CompositionScaleX, sender->CompositionScaleY);
                        auto size = DirectX::XMFLOAT2((float)sender->ActualWidth, (float)sender->ActualHeight);

                        this->TryResize(size, scale, dpi, orientation);
                    });

                this->sizeChangedToken = this->swapChainPanelXaml->SizeChanged += ref new Windows::UI::Xaml::SizeChangedEventHandler(
                    [=](Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ args)
                    {
                        LOG_FUNCTION_ENTER("displayInformation->SizeChanged()");
                        auto displayInformation = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
                        auto size = DirectX::XMFLOAT2(args->NewSize.Width, args->NewSize.Height);

                        concurrency::critical_section::scoped_lock lk(this->cs);
                        auto orientation = displayInformation->CurrentOrientation;
                        auto dpi = displayInformation->LogicalDpi;
                        auto scale = DirectX::XMFLOAT2(this->swapChainPanelXaml->CompositionScaleX, this->swapChainPanelXaml->CompositionScaleY);

                        this->TryResize(size, scale, dpi, orientation);
                    });
            });

        this->renderer.OutputParametersChanged();

        this->StartRenderThread();
        this->inputThread = std::thread([=] { 
            this->Input(this->swapChainPanelXaml);
            });
    }

    ~WinRtRenderer() {
        static_assert(std::is_base_of<IRenderer, T>::value, "Renderer must inherit from IRenderer");

        this->renderThreadState = RenderThreadState::Stop;
        this->coreInput->Dispatcher->StopProcessEvents();

        if (this->renderThread.joinable()) {
            this->renderThread.join();
        }

        if (this->inputThread.joinable()) {
            this->inputThread.join();
        }

        try {
            H::RunOnUIThread(this->swapChainPanelXaml->Dispatcher,
                [&]
                {
                    auto window = Windows::UI::Xaml::Window::Current->CoreWindow;
                    auto displayInformation = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();

                    window->VisibilityChanged -= this->visChangedToken;
                    displayInformation->OrientationChanged -= this->orientChangedToken;
                    displayInformation->DisplayContentsInvalidated -= this->dispContentInvalidatedToken;
                    displayInformation->DpiChanged -= this->dpiChangedToken;
                    this->swapChainPanelXaml->CompositionScaleChanged -= this->compScaleChangedToken;
                    this->swapChainPanelXaml->SizeChanged -= this->sizeChangedToken;
                });
        }
        catch (...) {}
    }

    T* operator->() {
        return &this->renderer;
    }

    T* get() {
        return &this->renderer;
    }

    DirectX::XMFLOAT4 GetRTColor();
    void SetRTColor(DirectX::XMFLOAT4 color);

    Helpers::WinRt::Dx::DxSettings^ GetDxSettings() {
        return this->swapChainPanelOutput.GetDxSettings();
    }

public:
    std::function<void(Windows::UI::Input::PointerPoint^)> pointerPressed;
    std::function<void(Windows::UI::Input::PointerPoint^)> pointerMoved;
    std::function<void(Windows::UI::Input::PointerPoint^)> pointerReleased;
    std::function<void(Windows::UI::Input::PointerPoint^)> pointerWheelChanged;

private:
    void Input(Windows::UI::Xaml::Controls::SwapChainPanel^ panel) {
        this->coreInput = swapChainPanelXaml->CreateCoreIndependentInputSource(
            Windows::UI::Core::CoreInputDeviceTypes::Mouse |
            Windows::UI::Core::CoreInputDeviceTypes::Touch |
            Windows::UI::Core::CoreInputDeviceTypes::Pen);

        this->coreInput->PointerPressed += H::System::MakeTypedEventHandler(
            [=](Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
            {
                auto pt = args->CurrentPoint;
                this->pointerMoves = true;

#ifdef WINRT_Input
                this->pointerPressed(pt);
#endif // WINRT_Input

            });
        this->coreInput->PointerMoved += H::System::MakeTypedEventHandler(
            [=](Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
            {
                if (this->pointerMoves) {
                    auto pt = args->CurrentPoint;

#ifdef WINRT_Input
                    this->pointerMoved(pt);
#endif // DEBUG

                }
            });
        this->coreInput->PointerReleased += H::System::MakeTypedEventHandler(
            [=](Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
            {
                auto pt = args->CurrentPoint;
                this->pointerMoves = false;

#ifdef WINRT_Input
                this->pointerReleased(pt);
#endif // WINRT_Input

            });
        this->coreInput->PointerWheelChanged += H::System::MakeTypedEventHandler(
            [=](Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ args)
            {
                auto pt = args->CurrentPoint;

#ifdef WINRT_Input
                this->pointerWheelChanged(pt);
#endif // WINRT_Input

            });

        this->coreInput->Dispatcher->ProcessEvents(Windows::UI::Core::CoreProcessEventsOption::ProcessUntilQuit);
    }

    void Render() {
        while (this->renderThreadState != RenderThreadState::Stop) {
            concurrency::critical_section::scoped_lock lk(this->cs);
            this->swapChainPanelOutput.Render([this] {
                this->renderer.Render();
                });
        }
    }


    void StartRenderThread() {
        if (this->renderThreadState != RenderThreadState::Work) {
            this->renderThreadState = RenderThreadState::Work;
            this->renderThread = std::thread([=]() {
                LOG_THREAD(L"Video render thread");
                this->Render();
                });
        }
    }

    // Need to inspect all parameters in order to use correct size parameters under windows 8.1
    // Because under 8.1 it isn't reports events about all changes, for example it refers to DPI
    void TryResize(
        const DirectX::XMFLOAT2& newSize,
        const DirectX::XMFLOAT2& newScale,
        float dpi,
        Windows::Graphics::Display::DisplayOrientations orientation)
    {
        LOG_FUNCTION_ENTER("TryResize(newSize = [{}; {}], newScale = [{}; {}], dpi = {}, ...)"
            , newSize.x, newSize.y
            , newScale.x, newScale.y
            , dpi
        );

        bool needResize = false;
        auto oldOrientation = this->swapChainPanelOutput.GetCurrentOrientation();
        auto oldDpi = this->swapChainPanelOutput.GetLogicalDpi();
        auto oldScale = this->swapChainPanelOutput.GetCompositionScale();
        auto oldSize = this->swapChainPanelOutput.GetLogicalSize();

        if (orientation != oldOrientation) {
            needResize = true;
            this->swapChainPanelOutput.SetCurrentOrientation(orientation);
        }

        if (dpi != oldDpi) {
            needResize = true;
            this->swapChainPanelOutput.SetLogicalDpi(dpi);
        }

        if (newScale != oldScale) {
            needResize = true;
            this->swapChainPanelOutput.SetCompositionScale(newScale);
        }

        if (newSize != oldSize) {
            needResize = true;
            this->swapChainPanelOutput.SetLogicalSize(newSize);
        }

        if (needResize) {
            this->renderer.OutputParametersChanged();
        }
    }

    private:
        Helpers::WinRt::Dx::SwapChainPanel^ swapChainPanelWinRt;
        Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml;
        SwapChainPanelOutput swapChainPanelOutput;
        T renderer;

        concurrency::critical_section cs;
        concurrency::critical_section renderThreadStateCs;
        
        bool pointerMoves;
        std::atomic<RenderThreadState> renderThreadState;

        Windows::UI::Xaml::Window^ wnd;
        Windows::UI::Core::CoreIndependentInputSource^ coreInput;
        Windows::Foundation::EventRegistrationToken visChangedToken;
        Windows::Foundation::EventRegistrationToken orientChangedToken;
        Windows::Foundation::EventRegistrationToken dispContentInvalidatedToken;
        Windows::Foundation::EventRegistrationToken dpiChangedToken;
        Windows::Foundation::EventRegistrationToken compScaleChangedToken;
        Windows::Foundation::EventRegistrationToken sizeChangedToken;

        std::thread renderThread;
        std::thread inputThread;
};

template<class T>
inline DirectX::XMFLOAT4 WinRtRenderer<T>::GetRTColor() {
    return this->swapChainPanelOutput.GetRTColor();
}

template<class T>
inline void WinRtRenderer<T>::SetRTColor(DirectX::XMFLOAT4 color) {
    this->swapChainPanelOutput.SetRTColor(color);
}

#endif // WINRT_Any