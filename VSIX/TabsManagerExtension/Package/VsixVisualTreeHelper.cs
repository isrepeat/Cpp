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


namespace TabsManagerExtension {
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