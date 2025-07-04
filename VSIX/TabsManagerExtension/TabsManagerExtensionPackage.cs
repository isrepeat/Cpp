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
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.VCProjectEngine;
using Microsoft.VisualStudio.VCCodeModel;
using Microsoft.VisualStudio.TextManager.Interop;
using Task = System.Threading.Tasks.Task;

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
            
            // TODO: adapt the CppFeatures nuget to net472 (WPF?)
            //var initFlags = CppFeatures.Cx.InitFlags.DefaultFlags | CppFeatures.Cx.InitFlags.CreateInPackageFolder;
            //CppFeatures.Cx.Logger.Init(AppConstants.LogFilename, initFlags);

            //Console.Beep(1000, 500); // 1000 Гц, 500 мс
            Services.ExtensionServices.Initialize();

            this.InitializeEvents();

            ToolWindows.EarlyPackageLoadHackToolWindow.Initialize(this);
            await ToolWindows.TabsManagerToolWindowCommand.InitializeAsync(this);
        }


        private void InitializeEvents() {
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded += this.OnSolutionLoaded;
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionClosed += this.OnSolutionClosed;
        }


        private void OnSolutionLoaded(string solutionName) {
            Helpers.Diagnostic.Logger.LogDebug($"[Package] OnSolutionLoaded(): solutionName = {solutionName}");
            PackageServices.Invalidate();

            if (VsixVisualTreeHelper.Instance.IsCustomTabsEnabled) {
                return;
            }

            Application.Current.Dispatcher.BeginInvoke(new Action(() => {
                VsixVisualTreeHelper.Instance.ToggleCustomTabs(true);
            }), DispatcherPriority.Background);
        }


        private void OnSolutionClosed(string solutionName) {
            Helpers.Diagnostic.Logger.LogDebug($"[Package] OnSolutionClosed(): solutionName = {solutionName}");
            PackageServices.Invalidate();

            if (!VsixVisualTreeHelper.Instance.IsCustomTabsEnabled) {
                return;
            }

            Application.Current.Dispatcher.BeginInvoke(new Action(() => {
                VsixVisualTreeHelper.Instance.ToggleCustomTabs(false);
            }), DispatcherPriority.Background);
        }
    }



    /// <summary>
    /// Централизованный сервис-локатор для получения COM-сервисов Visual Studio (IVsSolution, IVsUIShell, EnvDTE и др).
    /// Все свойства используют ленивую (lazy) инициализацию: сервис получается через Package.GetGlobalService только при первом доступе,
    /// и затем кешируется для повторных вызовов. При вызове Invalidate() кеш сбрасывается и сервисы будут получены заново при следующем доступе.
    /// </summary>
    public static class PackageServices {
        private static EnvDTE80.DTE2? _dte2;
        public static EnvDTE80.DTE2 Dte2 {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_dte2 == null) {
                    _dte2 = GetService<EnvDTE80.DTE2>(typeof(SDTE));
                }
                return _dte2;
            }
        }
        public static EnvDTE80.DTE2? TryGetDte2() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<EnvDTE80.DTE2>(typeof(SDTE));
        }


        private static IVsShell? _vsShell;
        public static IVsShell VsShell {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsShell == null) {
                    _vsShell = GetService<IVsShell>(typeof(SVsShell));
                }
                return _vsShell;
            }
        }
        public static IVsShell? TryGetVsShell() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsShell>(typeof(SVsShell));
        }


        private static IVsUIShell? _vsUIShell;
        public static IVsUIShell VsUIShell {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsUIShell == null) {
                    _vsUIShell = GetService<IVsUIShell>(typeof(SVsUIShell));
                }
                return _vsUIShell;
            }
        }
        public static IVsUIShell? TryGetVsUIShell() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsUIShell>(typeof(SVsUIShell));
        }


        private static IVsUIShellOpenDocument? _vsUIShellOpenDocument;
        public static IVsUIShellOpenDocument VsUIShellOpenDocument {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsUIShellOpenDocument == null) {
                    _vsUIShellOpenDocument = GetService<IVsUIShellOpenDocument>(typeof(SVsUIShellOpenDocument));
                }
                return _vsUIShellOpenDocument;
            }
        }
        public static IVsUIShellOpenDocument? TryGetVsUIShellOpenDocument() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsUIShellOpenDocument>(typeof(SVsUIShellOpenDocument));
        }


        private static IVsSolution? _vsSolution;
        public static IVsSolution VsSolution {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsSolution == null) {
                    _vsSolution = GetService<IVsSolution>(typeof(SVsSolution));
                }
                return _vsSolution;
            }
        }
        public static IVsSolution? TryGetVsSolution() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsSolution>(typeof(SVsSolution));
        }


        private static IVsTextManager? _vsTextManager;
        public static IVsTextManager VsTextManager {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsTextManager == null) {
                    _vsTextManager = GetService<IVsTextManager>(typeof(SVsTextManager));
                }
                return _vsTextManager;
            }
        }
        public static IVsTextManager? TryGetVsTestManager() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsTextManager>(typeof(SVsTextManager));
        }


        private static IVsMonitorSelection? _vsMonitorSelection;
        public static IVsMonitorSelection VsMonitorSelection {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsMonitorSelection == null) {
                    _vsMonitorSelection = GetService<IVsMonitorSelection>(typeof(SVsShellMonitorSelection));
                }
                return _vsMonitorSelection;
            }
        }
        public static IVsMonitorSelection? TryGetVsMonitorSelection() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsMonitorSelection>(typeof(SVsShellMonitorSelection));
        }


        private static IVsRunningDocumentTable? _vsRunningDocumentTable;
        public static IVsRunningDocumentTable VsRunningDocumentTable {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsRunningDocumentTable == null) {
                    _vsRunningDocumentTable = GetService<IVsRunningDocumentTable>(typeof(SVsRunningDocumentTable));
                }
                return _vsRunningDocumentTable;
            }
        }
        public static IVsRunningDocumentTable? TryGetVsRunningDocumentTable() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsRunningDocumentTable>(typeof(SVsRunningDocumentTable));
        }


        private static IVsTrackProjectDocuments2? _vsTrackProjectDocuments2;
        public static IVsTrackProjectDocuments2 VsTrackProjectDocuments2 {
            get {
                ThreadHelper.ThrowIfNotOnUIThread();

                if (_vsTrackProjectDocuments2 == null) {
                    _vsTrackProjectDocuments2 = GetService<IVsTrackProjectDocuments2>(typeof(SVsTrackProjectDocuments));
                }
                return _vsTrackProjectDocuments2;
            }
        }
        public static IVsTrackProjectDocuments2? TryGetVsTrackProjectDocuments2() {
            ThreadHelper.ThrowIfNotOnUIThread();
            return TryGetService<IVsTrackProjectDocuments2>(typeof(SVsTrackProjectDocuments));
        }



        /// <summary>
        /// Сбрасывает все кешированные ссылки, при следующем доступе они будут получены заново через Package.GetGlobalService.
        /// Использовать например при событиях загрузки / выгрузки решения.
        /// </summary>
        public static void Invalidate() {
            _dte2 = null;
            _vsShell = null;
            _vsUIShell = null;
            _vsUIShellOpenDocument = null;
            _vsSolution = null;
            _vsTextManager = null;
            _vsMonitorSelection = null;
            _vsRunningDocumentTable = null;
            _vsTrackProjectDocuments2 = null;
        }


        private static T GetService<T>(Type type) where T : class {
            ThreadHelper.ThrowIfNotOnUIThread();

            var service = Package.GetGlobalService(type) as T;
            if (service == null) {
                throw new InvalidOperationException($"Cannot get service {type.FullName}");
            }
            return service;
        }

        private static T? TryGetService<T>(Type type) where T : class {
            ThreadHelper.ThrowIfNotOnUIThread();
            return Package.GetGlobalService(type) as T;
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
            if (_currentTabHost == null || !_currentTabHost.TryGetTarget(out var knownHost) || knownHost != tabHost) {
                _originalTabListHostContent = tabHost.Child;
                _currentTabHost = new WeakReference<Decorator>(tabHost);
            }

            if (enable) {
                if (tabHost.Child is Controls.TabsManagerToolWindowControl) {
                    return; // Уже вставлено
                }
                
                //Services.ExtensionServices.Initialize();

                var customControl = new Controls.TabsManagerToolWindowControl();
                customControl.Unloaded += this.OnInjectedControlUnloaded;

                tabHost.Child = customControl;
                _lastInjectedContent = new WeakReference<UIElement>(customControl);

                _isCustomTabsEnabled = true;
                Helpers.Diagnostic.Logger.LogDebug("TabsManagerToolWindowControl injected.");
            }
            else {
                if (_originalTabListHostContent != null) {
                    tabHost.Child = _originalTabListHostContent;
                    _lastInjectedContent = null;

                    _isCustomTabsEnabled = false;
                    Helpers.Diagnostic.Logger.LogDebug("Restored original tab content.");

                    //Services.ExtensionServices.RequestShutdown();
                }
            }
        }

        /// <summary>
        /// Автоматическое переключение между оригинальным и кастомным таб-контролом.
        /// </summary>
        public void ToggleCustomTabs() {
            this.ToggleCustomTabs(!_isCustomTabsEnabled);
        }

        private void OnInjectedControlUnloaded(object sender, RoutedEventArgs e) {
            if (sender is FrameworkElement element) {
                element.Unloaded -= this.OnInjectedControlUnloaded;
            }

            Helpers.Diagnostic.Logger.LogDebug("TabsManagerToolWindowControl.Unloaded — re-evaluating state...");

            Application.Current.Dispatcher.BeginInvoke(new Action(() => {
                if (_isCustomTabsEnabled) {
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