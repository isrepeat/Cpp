using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using WinRT_DLL;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace TestUWP
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
            WinRT_DLL.CrashHandlerWinRt.CreateInstance("08792b7d-e0ca-47e6-a61f-a23f814c0685", "1.0.0.1", "00000000-0000-0000-0000-000000000001", "test-minidumpwriter");
            WinRT_DLL.CrashHandlerWinRt.GetInstance().CrashEvent += this.CrashHandler;
            
            var crasher = new WinRT_DLL.Crasher();
            int xxx = 9;
            crasher.AccessViolation();
            int yyy = 9;
        }

        private async void CrashHandler()
        {
            // ...
        }
    }
}
