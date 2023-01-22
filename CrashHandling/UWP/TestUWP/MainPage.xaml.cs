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
            var instance = new Class1();
            //TestCall();
        }

        private /*async*/ void TestCall() {
            var installFolder = Windows.ApplicationModel.Package.Current.InstalledLocation.Path;
            System.Diagnostics.Process newProcess = System.Diagnostics.Process.Start(installFolder + "\\MinidumpWriter.exe", "asdadasd");

            //if (Windows.Foundation.Metadata.ApiInformation.IsApiContractPresent("Windows.ApplicationModel.FullTrustAppContract", 1, 0))
            //{
            //    Windows.Storage.ApplicationData.Current.LocalSettings.Values["param_1"] = "11111";
            //    Windows.Storage.ApplicationData.Current.LocalSettings.Values["param_2"] = "22222";
            //    Windows.Storage.ApplicationData.Current.LocalSettings.Values["param_3"] = "33333";
            //    await Windows.ApplicationModel.FullTrustProcessLauncher.LaunchFullTrustProcessForCurrentAppAsync();
            //}
        }
    }
}
