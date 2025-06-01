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
    internal sealed class TabsManagerToolWindowCommand {
        public const int CommandId = 0x0100;

        public static readonly Guid CommandSet = new Guid("8a30806a-edfc-4c91-8182-025665145a07");

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

        public static TabsManagerToolWindowCommand Instance {
            get;
            private set;
        }

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


        private void Execute(object sender, EventArgs e) {
            ThreadHelper.ThrowIfNotOnUIThread();

            EarlyPackageLoadHackToolWindow.Instance.TEST_MoveToSmth();

            ////if (VsixVisualTreeHelper.IsCustomTabsInjected) {
            ////    VsixVisualTreeHelper.RestoreOriginalTabs();
            ////}
            ////else {
            ////    //VsixVisualTreeHelper.ScheduleInjectionTabsManagerControl();
            ////    VsixVisualTreeHelper.TryInject();
            ////}

            //// Get the instance number 0 of this tool window. This window is single instance so this instance
            //// is actually the only one.
            //// The last flag is set to true so that if the tool window does not exists it will be created.
            //ToolWindowPane window = this.package.FindToolWindow(typeof(TabsManagerToolWindow), 0, true);
            //if ((null == window) || (null == window.Frame)) {
            //    throw new NotSupportedException("Cannot create tool window");
            //}

            //IVsWindowFrame windowFrame = (IVsWindowFrame)window.Frame;
            //Microsoft.VisualStudio.ErrorHandler.ThrowOnFailure(windowFrame.Show());
        }
    }


    //internal sealed class TabsManagerToolWindowCommand {
    //    public const int CommandId = 0x0100;
    //    public static readonly Guid CommandSet = new Guid("8a30806a-edfc-4c91-8182-025665145a07");

    //    private readonly Package package;

    //    private TabsManagerToolWindowCommand(Package package, OleMenuCommandService commandService) {
    //        this.package = package ?? throw new ArgumentNullException(nameof(package));
    //        commandService = commandService ?? throw new ArgumentNullException(nameof(commandService));

    //        var menuCommandID = new CommandID(CommandSet, CommandId);
    //        var menuItem = new MenuCommand(this.Execute, menuCommandID);
    //        commandService.AddCommand(menuItem);
    //    }

    //    public static TabsManagerToolWindowCommand Instance { get; private set; }

    //    public static void Initialize(Package package) {
    //        ThreadHelper.ThrowIfNotOnUIThread();

    //        var commandService = (OleMenuCommandService)((IServiceProvider)package).GetService(typeof(IMenuCommandService));
    //        Instance = new TabsManagerToolWindowCommand(package, commandService);
    //    }

    //    private void Execute(object sender, EventArgs e) {
    //        ThreadHelper.ThrowIfNotOnUIThread();

    //        if (VsixVisualTreeHelper.IsCustomTabsInjected) {
    //            VsixVisualTreeHelper.RestoreOriginalTabs();
    //        }
    //        else {
    //            VsixVisualTreeHelper.TryInject();
    //        }
    //    }
    //}
}