using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Threading;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.ComponentModel.Design;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Task = System.Threading.Tasks.Task;
using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio;


namespace TabsManagerExtension {

    [PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
    [ProvideAutoLoad(UIContextGuids80.SolutionExists, PackageAutoLoadFlags.BackgroundLoad)]
    [ProvideAutoLoad(UIContextGuids80.NoSolution, PackageAutoLoadFlags.BackgroundLoad)]
    [ProvideMenuResource("Menus.ctmenu", 1)]
    [ProvideToolWindow(typeof(EarlyPackageLoadHackToolWindow))]
    [ProvideToolWindow(typeof(TabsManagerToolWindow))]
    [Guid(TabsManagerExtensionPackage.PackageGuidString)]
    public sealed class TabsManagerExtensionPackage : AsyncPackage {
        public const string PackageGuidString = "7a0ce045-e2ba-4f14-8b80-55cfd666e3d8";
        private const string OptionKey = "TabsManagerExtension";

        protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress) {
            // When initialized asynchronously, the current thread may be a background thread at this point.
            // Do any initialization that requires the UI thread after switching to the UI thread.
            await this.JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);

            // TODO: adapt the CppFeatures nuget to net472 (WPF?)
            //var initFlags = CppFeatures.Cx.InitFlags.DefaultFlags | CppFeatures.Cx.InitFlags.CreateInPackageFolder;
            //CppFeatures.Cx.Logger.Init(AppConstants.LogFilename, initFlags);

            EarlyPackageLoadHackToolWindow.Initialize(this);

            await TabsManagerToolWindowCommand.InitializeAsync(this);
        }
    }



    public static class VsixVisualTreeHelper {
        private static DispatcherTimer? _timer;
        private static UIElement? _originalTabListHostContent;
        private static Decorator? _tabHostDecorator;

        public static bool IsCustomTabsInjected {
            get {
                return _tabHostDecorator?.Child is TabsManagerToolWindowControl;
            }
        }

        public static void ScheduleInjectionTabsManagerControl() {
            _timer = new DispatcherTimer {
                Interval = TimeSpan.FromMilliseconds(500)
            };

            _timer.Tick += (s, e) => {
                TryInject();
            };

            _timer.Start();
        }

        public static void TryInject() {
            //Application.Current.Dispatcher.InvokeAsync(() => {
            var mainWindow = Application.Current.MainWindow;
            if (mainWindow == null) {
                return;
            }

            var tabHost = Helpers.VisualTree.FindElementByName(mainWindow, "PART_TabListHost");
            if (tabHost is Decorator decorator) {
                if (_originalTabListHostContent == null) {
                    _originalTabListHostContent = decorator.Child;
                    _tabHostDecorator = decorator;
                }

                decorator.Child = new TestTabsControl();
                //decorator.Child = new TabsManagerToolWindowControl();

                _timer?.Stop();
                _timer = null;
            }
            else {
                Helpers.Diagnostic.Logger.LogWarning($"tabHost not found");
            }
            //}, DispatcherPriority.Loaded);
        }

        public static void RestoreOriginalTabs() {
            if (_tabHostDecorator != null && _originalTabListHostContent != null) {
                _tabHostDecorator.Child = _originalTabListHostContent;
            }
        }
    }
}