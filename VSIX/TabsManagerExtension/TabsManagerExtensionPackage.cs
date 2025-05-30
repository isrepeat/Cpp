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

namespace TabsManagerExtension {
    [PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
    [Guid(TabsManagerExtensionPackage.PackageGuidString)]
    [ProvideMenuResource("Menus.ctmenu", 1)]
    [ProvideToolWindow(typeof(TabsManagerToolWindow))]
    [ProvideAutoLoad(UIContextGuids80.SolutionExists, PackageAutoLoadFlags.BackgroundLoad)]
    public sealed class TabsManagerExtensionPackage : AsyncPackage {
        public const string PackageGuidString = "7a0ce045-e2ba-4f14-8b80-55cfd666e3d8";

        protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress) {
            // When initialized asynchronously, the current thread may be a background thread at this point.
            // Do any initialization that requires the UI thread after switching to the UI thread.
            await this.JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);

            // TODO: adapt the CppFeatures nuget to net472 (WPF?)
            //var initFlags = CppFeatures.Cx.InitFlags.DefaultFlags | CppFeatures.Cx.InitFlags.CreateInPackageFolder;
            //CppFeatures.Cx.Logger.Init(AppConstants.LogFilename, initFlags);

#if __REPLACE_SRC_TABS
            VsixVisualTreeHelper.ScheduleInjectionTabsManagerControl();
#else
            await TabsManagerToolWindowCommand.InitializeAsync(this);
#endif
        }
    }


    public static class VsixVisualTreeHelper {
        private static DispatcherTimer? _timer;
        public static void ScheduleInjectionTabsManagerControl() {
            _timer = new DispatcherTimer {
                Interval = TimeSpan.FromMilliseconds(500)
            };

            _timer.Tick += (s, e) =>
            {
                TryInject();
            };

            _timer.Start();
        }

        private static void TryInject() {
            var mainWindow = Application.Current.MainWindow;
            if (mainWindow == null) {
                return;
            }

            var tabHost = Helpers.VisualTree.FindElementByName(mainWindow, "PART_TabListHost");
            if (tabHost is Decorator decorator) {
                decorator.Child = new TabsManagerToolWindowControl();

                _timer?.Stop();
                _timer = null;
            }


            //if (tabHost is FrameworkElement fe) {
            //    var parent = VisualTreeHelper.GetParent(fe) as Panel;
            //    if (parent != null) {
            //        int index = parent.Children.IndexOf(fe);
            //        if (index >= 0) {
            //            var customControl = new TabsManagerToolWindowControl();
            //            parent.Children.RemoveAt(index);
            //            //parent.Children.Insert(index, customControl);

            //            //Debug.WriteLine("[TabsInjector] Успешно заменили PART_TabListHost");

            //            //// Доп. проверка через Dispatcher — остался ли контрол
            //            //Application.Current.Dispatcher.InvokeAsync(async () =>
            //            //{
            //            //    await Task.Delay(500);

            //            //    bool stillThere = parent.Children.Contains(customControl);
            //            //    Debug.WriteLine(stillThere
            //            //        ? "[TabsInjector] Контрол остался на месте — замена успешна ✅"
            //            //        : "[TabsInjector] Контрол заменён обратно — замена провалилась ❌");
            //            //});

            //            // Останавливаем таймер
            //            _timer?.Stop();
            //            _timer = null;
            //        }
            //    }
            //}
        }
    }
}