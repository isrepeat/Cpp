using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using Microsoft.Web.WebView2.Core;
using SimpleApp.WinUI3.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;


namespace SimpleApp.WinUI3 {
    public class IntercomHtmlWidget {
        private bool intercomNavigationCompleted = false;
        private StorageFile intercomHtmlFile = null;

        // TODO: use weak ref.
        private WebView2 webView;

        public IntercomHtmlWidget(WebView2 webView) {
            this.webView = webView;
            this.webView.NavigationCompleted += OnNavigationCompleted;
            this.InitializeAsync();
        }

        private async void InitializeAsync() {
            this.intercomHtmlFile = await StorageFile.GetFileFromApplicationUriAsync(new Uri("ms-appx:///Assets/Html/Intercom.html"));
        }

        public void Navigate() {
            if (!this.intercomNavigationCompleted) {
                this.webView.CoreWebView2.Navigate($"file:///{this.intercomHtmlFile.Path}");
            }
        }

        private void OnNavigationCompleted(object sender, CoreWebView2NavigationCompletedEventArgs e) {
            var navigatedUri = this.webView.Source;
            Diagnostic.Logger.LogDebug($"[IntercomHtmlWidget] Navigation completed: {navigatedUri}");

            if (navigatedUri.IsFile) {
                string fileName = Path.GetFileName(navigatedUri.LocalPath);
                if (fileName == "Intercom.html") {
                    this.intercomNavigationCompleted = true;
                }
            }
        }
    }

    public sealed partial class MainWindow_5_Intercom : Window {
        private IntercomHtmlWidget intercomHtmlWidget;
        public MainWindow_5_Intercom() {
            this.InitializeComponent();
            this.InitializeAsync();
        }

        private async void InitializeAsync() {
            await this.InitializeWebView();
        }

        private async Task InitializeWebView() {
            await this.MyWebView.EnsureCoreWebView2Async();
            this.MyWebView.NavigationCompleted += OnNavigationCompleted;

            this.intercomHtmlWidget = new IntercomHtmlWidget(this.MyWebView);
        }

        private void OnNavigationCompleted(object sender, CoreWebView2NavigationCompletedEventArgs e) {
            var navigatedUri = this.MyWebView.Source;
            Diagnostic.Logger.LogDebug($"Navigation completed, navigatedUri = {navigatedUri}");
        }

        private void MyWebView_LostFocus(object sender, RoutedEventArgs e) {
            this.MyWebView.Visibility = Visibility.Collapsed;
        }

        private void myButton_Click(object sender, RoutedEventArgs e) {
            this.MyWebView.Visibility = Visibility.Visible;
            this.MyWebView.Focus(FocusState.Programmatic);

            if (this.MyWebView.Visibility == Visibility.Visible) {
                this.intercomHtmlWidget.Navigate();
            }
        }
    }
}