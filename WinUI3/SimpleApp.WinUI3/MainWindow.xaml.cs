// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace SimpleApp.WinUI3 {
    using Microsoft.UI.Xaml;
    using Microsoft.UI.Xaml.Controls;
    using Microsoft.UI.Xaml.Controls.Primitives;
    using Microsoft.UI.Xaml.Data;
    using Microsoft.UI.Xaml.Input;
    using Microsoft.UI.Xaml.Media;
    using Microsoft.UI.Xaml.Navigation;
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices.WindowsRuntime;
    using Windows.Foundation;
    using Windows.Foundation.Collections;

    using TestLibrary_WinRt_WinUI3;

    /// <summary>
    /// An empty window that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainWindow : Window {
        public MainWindow() {
            this.InitializeComponent();


        }
        private void myButton_Click(object sender, RoutedEventArgs e) {
            var mainTypesWinRt = new MainTypesWinRt();

            var dxSettings = mainTypesWinRt.GetDxSettings();
            bool msaa = dxSettings.MSAA;

            var testIntProperty = mainTypesWinRt.TestIntProperty;

            var testStringProperty = mainTypesWinRt.TestStringProperty;
            mainTypesWinRt.TestStringProperty = "string modified";
            testStringProperty = mainTypesWinRt.TestStringProperty;

            TestAsync(mainTypesWinRt);

            var testStringPropertyOnlyGet = mainTypesWinRt.TestStringPropertyOnlyGet;

            myButton.Content = "Clicked";
        }


        private void ProgressEventHandler(object sender, float progress) {
            float x = progress;
        }

        private async void TestAsync(MainTypesWinRt mainTypesWinRt) {
            await mainTypesWinRt.TestAsyncAction();

            mainTypesWinRt.ProgressEvent += ProgressEventHandler;
            mainTypesWinRt.DoSomething();
        }
    }
}
