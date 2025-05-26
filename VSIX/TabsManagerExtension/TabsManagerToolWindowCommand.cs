namespace TabsManagerExtension {
    using Microsoft.VisualStudio.Shell;
    using Microsoft.VisualStudio.Shell.Interop;
    using System;
    using System.ComponentModel.Design;
    using System.Globalization;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows.Media;
    using System.Windows;
    using Task = System.Threading.Tasks.Task;
    using System.Windows.Controls;
    using System.Linq;

    /// <summary>
    /// Command handler
    /// </summary>
    internal sealed class TabsManagerToolWindowCommand {
        /// <summary>
        /// Command ID.
        /// </summary>
        public const int CommandId = 0x0100;

        /// <summary>
        /// Command menu group (command set GUID).
        /// </summary>
        public static readonly Guid CommandSet = new Guid("8a30806a-edfc-4c91-8182-025665145a07");

        /// <summary>
        /// VS Package that provides this command, not null.
        /// </summary>
        private readonly AsyncPackage package;

        /// <summary>
        /// Initializes a new instance of the <see cref="TabsManagerToolWindowCommand"/> class.
        /// Adds our command handlers for menu (commands must exist in the command table file)
        /// </summary>
        /// <param name="package">Owner package, not null.</param>
        /// <param name="commandService">Command service to add command to, not null.</param>
        private TabsManagerToolWindowCommand(AsyncPackage package, OleMenuCommandService commandService) {
            this.package = package ?? throw new ArgumentNullException(nameof(package));
            commandService = commandService ?? throw new ArgumentNullException(nameof(commandService));

            var menuCommandID = new CommandID(CommandSet, CommandId);
            var menuItem = new MenuCommand(this.Execute, menuCommandID);
            commandService.AddCommand(menuItem);
        }

        /// <summary>
        /// Gets the instance of the command.
        /// </summary>
        public static TabsManagerToolWindowCommand Instance {
            get;
            private set;
        }

        /// <summary>
        /// Gets the service provider from the owner package.
        /// </summary>
        private Microsoft.VisualStudio.Shell.IAsyncServiceProvider ServiceProvider {
            get {
                return this.package;
            }
        }

        /// <summary>
        /// Initializes the singleton instance of the command.
        /// </summary>
        /// <param name="package">Owner package, not null.</param>
        public static async Task InitializeAsync(AsyncPackage package) {
            // Switch to the main thread - the call to AddCommand in TabsManagerToolWindowCommand's constructor requires
            // the UI thread.
            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync(package.DisposalToken);

            OleMenuCommandService commandService = await package.GetServiceAsync(typeof(IMenuCommandService)) as OleMenuCommandService;
            Instance = new TabsManagerToolWindowCommand(package, commandService);
        }

        /// <summary>
        /// Shows the tool window when the menu item is clicked.
        /// </summary>
        /// <param name="sender">The event sender.</param>
        /// <param name="e">The event args.</param>
        //private void Execute(object sender, EventArgs e) {
        //    ThreadHelper.ThrowIfNotOnUIThread();

        //    // Get the instance number 0 of this tool window. This window is single instance so this instance
        //    // is actually the only one.
        //    // The last flag is set to true so that if the tool window does not exists it will be created.
        //    ToolWindowPane window = this.package.FindToolWindow(typeof(TabsManagerToolWindow), 0, true);
        //    if ((null == window) || (null == window.Frame)) {
        //        throw new NotSupportedException("Cannot create tool window");
        //    }

        //    IVsWindowFrame windowFrame = (IVsWindowFrame)window.Frame;
        //    Microsoft.VisualStudio.ErrorHandler.ThrowOnFailure(windowFrame.Show());
        //}



        //private void Execute(object sender, EventArgs e) {
        //    var mainWindow = Application.Current.MainWindow;
        //    if (mainWindow == null) return;
        //    HideTabControls(mainWindow);
        //}

        ////private void HideTabControls(DependencyObject parent) {
        ////    for (int i = 0; i < VisualTreeHelper.GetChildrenCount(parent); i++) {
        ////        var child = VisualTreeHelper.GetChild(parent, i);
        ////        var type = child.GetType().FullName;

        ////        System.Diagnostics.Debug.WriteLine(type);

        ////        if (child is TabControl tabControl) {
        ////            // Будь осторожен — это может быть не "Tabs", а весь редактор
        ////            tabControl.Opacity = 0.3;
        ////        }
        ////        else {
        ////            HideTabControls(child);
        ////        }
        ////    }
        ////}

        //private void HideTabControls(DependencyObject parent) {
        //    for (int i = 0; i < VisualTreeHelper.GetChildrenCount(parent); i++) {
        //        var child = VisualTreeHelper.GetChild(parent, i);
        //        var type = child.GetType().FullName;

        //        // Целевые "вкладочные" компоненты
        //        if (type == "Microsoft.VisualStudio.Platform.WindowManagement.ContentFreeTabControl"
        //            || type == "Microsoft.VisualStudio.Platform.WindowManagement.DocumentGroupTabList"
        //            || type == "Microsoft.VisualStudio.Platform.WindowManagement.DocumentGroupContainerTabList") {
        //            if (child is UIElement ui) {
        //                ui.Visibility = Visibility.Collapsed;
        //            }
        //        }
        //        else {
        //            HideTabControls(child);
        //        }
        //    }
        //}


        private void Execute(object sender, EventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var dte = (EnvDTE.DTE)Package.GetGlobalService(typeof(EnvDTE.DTE));
            var mainWindow = Application.Current.MainWindow;
            if (mainWindow == null || dte == null) return;

            var tabHost = FindTabHost(mainWindow);
            if (tabHost == null) return;

            tabHost.Visibility = Visibility.Collapsed;

            var parentPanel = FindInsertTarget(tabHost);
            if (parentPanel == null) return;

            //// Удалим старую вставку, если она уже есть
            //var existing = parentPanel.Children.OfType<TabsManagerToolWindowControl>().FirstOrDefault();
            //if (existing != null)
            //    parentPanel.Children.Remove(existing);

            // Вставляем свой UserControl
            var control = new TabsManagerToolWindowControl();
            parentPanel.Children.Add(control);
        }

        private FrameworkElement? FindTabHost(DependencyObject parent) {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(parent); i++) {
                var child = VisualTreeHelper.GetChild(parent, i);
                var type = child.GetType().FullName;

                //if (type == "Microsoft.VisualStudio.Platform.WindowManagement.ContentFreeTabControl") {
                if (type == "Microsoft.VisualStudio.Platform.WindowManagement.DocumentGroupContainerTabList") {
                    return child as FrameworkElement;
                }

                var result = FindTabHost(child);
                if (result != null)
                    return result;
            }

            return null;
        }

        private Panel? FindInsertTarget(DependencyObject start) {
            var current = start;
            while (current != null) {
                if (current is Panel panel)
                    return panel;

                current = VisualTreeHelper.GetParent(current);
            }
            return null;
        }


        private void InjectCustomTabsUI(Panel parentPanel, EnvDTE.DTE dte) {
            var tabBar = new StackPanel {
                Orientation = Orientation.Horizontal,
                Background = new SolidColorBrush(Color.FromRgb(30, 30, 30)),
                Height = 28,
                Margin = new Thickness(0, 0, 0, 1)
            };

            foreach (EnvDTE.Document doc in dte.Documents) {
                var fileName = System.IO.Path.GetFileName(doc.FullName);
                var btn = new Button {
                    Content = fileName,
                    Margin = new Thickness(4, 2, 4, 2),
                    Padding = new Thickness(8, 0, 8, 0),
                    Background = Brushes.Transparent,
                    Foreground = Brushes.White,
                    BorderBrush = Brushes.Gray,
                    BorderThickness = new Thickness(0, 0, 0, 1),
                    FontSize = 12
                };

                btn.Click += (s, e) => { doc.Activate(); };

                tabBar.Children.Add(btn);
            }

            parentPanel.Children.Add(tabBar);
        }
    }
}