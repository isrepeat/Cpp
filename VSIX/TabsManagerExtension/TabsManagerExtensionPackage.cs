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

#if NET_FRAMEWORK_472
namespace System.Runtime.CompilerServices {
    internal static class IsExternalInit { } // need for "init" keyword
}
#endif

namespace TabsManagerExtension {
    /// <summary>
    /// Благодаря <b>[ProvideAutoLoad(UIContextGuids80.SolutionExists, PackageAutoLoadFlags.BackgroundLoad)]</b>
    /// новоустановленный пакет загрузиться в бэкграунде (через ~5c), после загрузки решения.
    /// Далее мы используем EarlyPackageLoadHackToolWindow, чтобы пакет загружался сразу при запуске VS.
    /// </summary>
    [PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
    [ProvideAutoLoad(UIContextGuids80.SolutionExists, PackageAutoLoadFlags.BackgroundLoad)]
    [ProvideMenuResource("Menus.ctmenu", 1)]
    [ProvideToolWindow(typeof(ToolWindows.EarlyPackageLoadHackToolWindow))]
    [ProvideToolWindow(typeof(ToolWindows.TabsManagerToolWindow))]
    [Guid(TabsManagerExtensionPackage.PackageGuidString)]
    public sealed class TabsManagerExtensionPackage : AsyncPackage {
        public const string PackageGuidString = "7a0ce045-e2ba-4f14-8b80-55cfd666e3d8";
        private const string OptionKey = "TabsManagerExtension";

        protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress) {
            // When initialized asynchronously, the current thread may be a background thread at this point.
            // Do any initialization that requires the UI thread after switching to the UI thread.
            await this.JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);
            
            Console.Beep(1000, 500); // 1000 Гц, 500 мс
            Services.ExtensionServices.Initialize();

            // TODO: adapt the CppFeatures nuget to net472 (WPF?)
            //var initFlags = CppFeatures.Cx.InitFlags.DefaultFlags | CppFeatures.Cx.InitFlags.CreateInPackageFolder;
            //CppFeatures.Cx.Logger.Init(AppConstants.LogFilename, initFlags);

            ToolWindows.EarlyPackageLoadHackToolWindow.Initialize(this); // Call VsixVisualTreeHelper.ToggleCustomTabs(true) inside.
            await ToolWindows.TabsManagerToolWindowCommand.InitializeAsync(this);
        }
    }



    public class VsixVisualTreeHelper : Helpers.ObservableObject {
        private static readonly VsixVisualTreeHelper _instance = new();
        public static VsixVisualTreeHelper Instance => _instance;

        public bool IsCustomTabsInjected {
            get {
                return _currentTabHost?.TryGetTarget(out var decorator) == true &&
                       decorator.Child is Controls.TabsManagerToolWindowControl;
            }
        }

        private bool _isCustomTabsEnabled = false;
        public bool IsCustomTabsEnabled {
            get => _isCustomTabsEnabled;
            private set {
                if (_isCustomTabsEnabled != value) {
                    _isCustomTabsEnabled = value;
                    OnPropertyChanged();
                }
            }
        }


        private UIElement? _originalTabListHostContent;
        private WeakReference<Decorator>? _currentTabHost;
        private WeakReference<UIElement>? _lastInjectedContent;

        private VsixVisualTreeHelper() {
        }

        /// <summary>
        /// Переключает отображение между оригинальным содержимым PART_TabListHost и кастомным контролом.
        /// </summary>
        /// <param name="enable">Если true — включить кастомные вкладки, иначе вернуть оригинал.</param>
        public void ToggleCustomTabs(bool enable) {
            using var __logFunctionScoped = Helpers.Diagnostic.Logger.LogFunctionScope($"ToggleCustomTabs({enable})");

            var mainWindow = Application.Current.MainWindow;
            if (mainWindow == null) {
                return;
            }

            var tabHost = Helpers.VisualTree.FindElementByName(mainWindow, "PART_TabListHost") as Decorator;
            if (tabHost == null) {
                Helpers.Diagnostic.Logger.LogWarning("PART_TabListHost not found");
                return;
            }

            // Если Decorator пересоздан — сбросим оригинальный контент
            if (this._currentTabHost == null || !this._currentTabHost.TryGetTarget(out var knownHost) || knownHost != tabHost) {
                this._originalTabListHostContent = tabHost.Child;
                this._currentTabHost = new WeakReference<Decorator>(tabHost);
            }

            if (enable) {
                if (tabHost.Child is Controls.TabsManagerToolWindowControl) {
                    return; // Уже вставлено
                }
                
                //Services.ExtensionServices.Initialize();

                var customControl = new Controls.TabsManagerToolWindowControl();
                customControl.Unloaded += this.OnInjectedControlUnloaded;

                tabHost.Child = customControl;
                this._lastInjectedContent = new WeakReference<UIElement>(customControl);

                this._isCustomTabsEnabled = true;
                Helpers.Diagnostic.Logger.LogDebug("TabsManagerToolWindowControl injected.");
            }
            else {
                if (this._originalTabListHostContent != null) {
                    tabHost.Child = this._originalTabListHostContent;
                    this._lastInjectedContent = null;

                    this._isCustomTabsEnabled = false;
                    Helpers.Diagnostic.Logger.LogDebug("Restored original tab content.");

                    //Services.ExtensionServices.RequestShutdown();
                }
            }
        }

        /// <summary>
        /// Автоматическое переключение между оригинальным и кастомным таб-контролом.
        /// </summary>
        public void ToggleCustomTabs() {
            this.ToggleCustomTabs(!this._isCustomTabsEnabled);
        }

        private void OnInjectedControlUnloaded(object sender, RoutedEventArgs e) {
            if (sender is FrameworkElement element) {
                element.Unloaded -= this.OnInjectedControlUnloaded;
            }

            Helpers.Diagnostic.Logger.LogDebug("TabsManagerToolWindowControl.Unloaded — re-evaluating state...");

            Application.Current.Dispatcher.BeginInvoke(new Action(() => {
                if (this._isCustomTabsEnabled) {
                    this.ToggleCustomTabs(true); // повторно инжектим
                }
            }), DispatcherPriority.Background);
        }
    }
}


namespace TabsManagerExtension.Behaviours {
    public class Dummy {
    }
}