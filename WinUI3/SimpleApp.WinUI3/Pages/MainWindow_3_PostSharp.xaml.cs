using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using SimpleApp.WinUI3.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using System.ComponentModel;
//using PostSharp.Aspects;
//using PostSharp.Serialization;


namespace SimpleApp.WinUI3 {
    //[PSerializable]
    //public class DefaultPropertyAspect : LocationInterceptionAspect {
    //    public override void OnSetValue(LocationInterceptionArgs args) {
    //        var oldValue = args.GetCurrentValue();
    //        var newValue = args.Value;
    //        Diagnostic.Logger.LogDebug($"Change property '{args.LocationFullName}' from '{oldValue}' to '{newValue}'");
    //        base.OnSetValue(args);
    //    }
    //}

    //[PSerializable]
    //public class DefaultNotifiablePropertyAspect : LocationInterceptionAspect {
    //    public override void OnSetValue(LocationInterceptionArgs args) {
    //        var oldValue = args.GetCurrentValue();
    //        var newValue = args.Value;
    //        Diagnostic.Logger.LogDebug($"Change property '{args.LocationFullName}' from '{oldValue}' to '{newValue}'");
    //        base.OnSetValue(args);

    //        if (!Equals(oldValue, newValue)) {
    //            var instance = args.Instance as INotifyPropertyChanged;
    //            instance?.GetType()
    //                     .GetMethod("OnPropertyChanged", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance)
    //                     ?.Invoke(instance, new object[] { args.LocationName });
    //        }
    //    }
    //}


    //[PSerializable]
    //public class CustomPropertyAspect : LocationInterceptionAspect {
    //    public override void OnSetValue(LocationInterceptionArgs args) {
    //        var oldValue = args.GetCurrentValue();
    //        var newValue = args.Value;
    //        Diagnostic.Logger.LogDebug($"Change property '{args.LocationFullName}' from '{oldValue}' to '{newValue}'");
    //        args.ProceedSetValue(); // Call original setter
    //    }
    //}

    //[PSerializable]
    //public class CustomNotifiablePropertyAspect : LocationInterceptionAspect {
    //    public override void OnSetValue(LocationInterceptionArgs args) {
    //        var oldValue = args.GetCurrentValue();
    //        var newValue = args.Value;
    //        Diagnostic.Logger.LogDebug($"Change property '{args.LocationFullName}' from '{oldValue}' to '{newValue}'");
    //        args.ProceedSetValue(); // Call original setter

    //        if (!Equals(oldValue, newValue)) {
    //            var instance = args.Instance as INotifyPropertyChanged;
    //            instance?.GetType()
    //                     .GetMethod("OnPropertyChanged", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance)
    //                     ?.Invoke(instance, new object[] { args.LocationName });
    //        }
    //    }
    //}


    public class MainWindow_3_PostSharp_ViewModel : Helpers.ObservableObject {

        //private string _fileURL = "";
        //[CustomNotifiablePropertyAspect]
        //public string FileURL {
        //    get {
        //        return _fileURL;
        //    }
        //    set {
        //        _fileURL = value;
        //        this.SomeMethod();
        //    }
        //}


        private void SomeMethod() {
        }

        public MainWindow_3_PostSharp_ViewModel() {
            _ = this.InitializeAsync();
        }

        private async Task InitializeAsync() {
            await Task.Delay(3000);
            //this.FileURL = "https://example.com";
            //this.FileURL = "https://another-example.com";
        }
    }

    public sealed partial class MainWindow_3_PostSharp : Window {
        private MainWindow_3_PostSharp_ViewModel viewModel;

        public MainWindow_3_PostSharp() {
            this.InitializeComponent();
            this.viewModel = new MainWindow_3_PostSharp_ViewModel();
            this.MainGrid.DataContext = this.viewModel;
        }
        private void myButton_Click(object sender, RoutedEventArgs e) {
        }
    }
}