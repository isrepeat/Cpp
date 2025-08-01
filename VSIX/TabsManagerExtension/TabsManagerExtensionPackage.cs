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

[assembly: Helpers.Attributes.CodeAnalyzerEnableLogs]

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
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded.Add(this.OnSolutionLoaded);
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionLoaded.InvokeForLastHandlerIfTriggered();

            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionClosed.Add(this.OnSolutionClosed);
            VsShell.Services.VsIDEStateFlagsTrackerService.Instance.SolutionClosed.InvokeForLastHandlerIfTriggered();
        }


        private void OnSolutionLoaded(string solutionName) {
            Helpers.Diagnostic.Logger.LogDebug($"[Package] OnSolutionLoaded(): solutionName = {solutionName}");
            PackageServices.Invalidate();

            if (VsixVisualTreeHelper.Instance.IsCustomTabsEnabled) {
                return;
            }

            VsixThreadHelper.RunOnVsThread(() => {
                VsixVisualTreeHelper.Instance.ToggleCustomTabs(true);
            });
        }


        private void OnSolutionClosed(string solutionName) {
            Helpers.Diagnostic.Logger.LogDebug($"[Package] OnSolutionClosed(): solutionName = {solutionName}");
            PackageServices.Invalidate();

            if (!VsixVisualTreeHelper.Instance.IsCustomTabsEnabled) {
                return;
            }

            VsixThreadHelper.RunOnVsThread(() => {
                VsixVisualTreeHelper.Instance.ToggleCustomTabs(false);
            });
        }
    }
}


namespace TabsManagerExtension.Behaviours {
    public class Dummy {
    }
}