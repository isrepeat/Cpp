using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Controls.Primitives;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Navigation;
using Microsoft.UI.Xaml.Shapes;
using SimpleApp.WinUI3.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Foundation;
using Windows.Foundation.Collections;

namespace SimpleApp.WinUI3 {

    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    public partial class App : Application {
        /// <summary>
        /// Initializes the singleton application object.  This is the first line of authored code
        /// executed, and as such is the logical equivalent of main() or WinMain().
        /// </summary>
        public App() {
            this.InitializeComponent();
            var initFlags = CppFeatures.Cx.InitFlags.DefaultFlags | CppFeatures.Cx.InitFlags.CreateInPackageFolder;
            CppFeatures.Cx.Logger.Init("Intublade.log", initFlags);


            // Registering a global handler for unhandled UI thread exceptions.
            this.UnhandledException += App_UnhandledException;

            // Registering a global handler for unhandled exceptions in asynchronous tasks.
            TaskScheduler.UnobservedTaskException += TaskScheduler_UnobservedTaskException;

            // Registering a global handler for unhandled exceptions in other threads.
            AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;

            // Registering a global handler for unhandled exceptions in asynchronous tasks.
            TaskScheduler.UnobservedTaskException += TaskScheduler_UnobservedTaskException;
        }

        private void App_UnhandledException(object sender, Microsoft.UI.Xaml.UnhandledExceptionEventArgs e) {
            this.LogException(e.Exception);
            e.Handled = true; // Prevent application termination.
        }

        private void TaskScheduler_UnobservedTaskException(object sender, UnobservedTaskExceptionEventArgs e) {
            this.LogException(e.Exception);
            e.SetObserved(); // Prevent application termination.
        }

        private void CurrentDomain_UnhandledException(object sender, System.UnhandledExceptionEventArgs e) {
            LogException(e.ExceptionObject as Exception);
            // e.Handled cannot be set here, so the application may terminate.
        }

        private void LogException(Exception ex) {
            Diagnostic.Logger.LogDebug($"Global catch unhandled exception = {ex?.Message}");
        }

        /// <summary>
        /// Invoked when the application is launched.
        /// </summary>
        /// <param name="args">Details about the launch request and process.</param>
        protected override void OnLaunched(Microsoft.UI.Xaml.LaunchActivatedEventArgs args) {
            //m_window = new MainWindow();
            //m_window = new MainWindow_1_AsyncLock();
            //m_window = new MainWindow_2_AsyncTimeout();
            //m_window = new MainWindow_3_PostSharp();
            //m_window = new MainWindow_4_CrashHandling();
            m_window = new MainWindow_5_Intercom();
            m_window.Activate();
        }

        private Window m_window;
    }
}