using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using System;
using System.ComponentModel.Design;
using System.Runtime.InteropServices;
using System.Threading;
using Task = System.Threading.Tasks.Task;

namespace HelloWorldExtension {
    [PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
    [Guid(HelloWorldExtensionPackage.PackageGuidString)]
    [ProvideMenuResource("Menus.ctmenu", 1)]
    public sealed class HelloWorldExtensionPackage : AsyncPackage {
        public const string PackageGuidString = "0089fa56-dad3-49b1-a8c5-c290ab558d29";

        // GUID и CommandID для стандартной команды "Закрепить вкладку"
        private static readonly Guid StandardCommandSetGuid = new Guid("5efc7975-14bc-11cf-9b2b-00aa00573819");
        private const int PinTabCommandID = 264;

        protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress) {
            // When initialized asynchronously, the current thread may be a background thread at this point.
            // Do any initialization that requires the UI thread after switching to the UI thread.
            await this.JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);
            await HelloWorldCommand.InitializeAsync(this);

            // Перехват стандартной команды "Закрепить вкладку"
            var commandService = await GetServiceAsync(typeof(IMenuCommandService)) as OleMenuCommandService;
            if (commandService != null) {
                var commandId = new CommandID(new Guid("5efc7975-14bc-11cf-9b2b-00aa00573819"), 264); // Стандартный GUID и Command ID
                var menuItem = new OleMenuCommand(OnPinTabClicked, commandId);
                menuItem.BeforeQueryStatus += OnBeforeQueryStatus;
                commandService.AddCommand(menuItem);
            }
        }

        private void OnBeforeQueryStatus(object sender, EventArgs e) {
            if (sender is OleMenuCommand command) {
                command.Visible = true;
                command.Enabled = true;
            }
        }

        private void OnPinTabClicked(object sender, EventArgs e) {
            var activeDocument = GetActiveDocumentName();
            VsShellUtilities.ShowMessageBox(
                this,
                activeDocument != null ? $"Вкладка перехвачена и закреплена: {activeDocument}" : "Нет активной вкладки",
                "Tab Pin Extension",
                OLEMSGICON.OLEMSGICON_INFO,
                OLEMSGBUTTON.OLEMSGBUTTON_OK,
                OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST
            );
        }

        private string GetActiveDocumentName() {
            ThreadHelper.ThrowIfNotOnUIThread();
            var monitorSelection = GetGlobalService(typeof(SVsShellMonitorSelection)) as IVsMonitorSelection;
            if (monitorSelection == null) return null;

            IntPtr hierarchyPtr = IntPtr.Zero;
            IntPtr selectionContainerPtr = IntPtr.Zero;
            IVsMultiItemSelect multiItemSelect = null;
            uint itemid = VSConstants.VSITEMID_NIL;

            monitorSelection.GetCurrentSelection(out hierarchyPtr, out itemid, out multiItemSelect, out selectionContainerPtr);

            if (hierarchyPtr == IntPtr.Zero) return null;

            IVsHierarchy hierarchy = Marshal.GetObjectForIUnknown(hierarchyPtr) as IVsHierarchy;
            hierarchy.GetCanonicalName(itemid, out string documentName);

            return documentName;
        }
    }
}