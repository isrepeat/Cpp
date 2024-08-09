#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/TestLibrary_WinRt_WinUI3.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::SimpleAppCpp_WinUI3::implementation
{
    int32_t MainWindow::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainWindow::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void MainWindow::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        auto instance = winrt::TestLibrary_WinRt_WinUI3::MainTypesWinRt{};
        auto dx = instance.GetDxSettings();
        auto msaa = dx.MSAA();
        myButton().Content(box_value(L"Clicked"));
    }
}
