using System;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using System.Windows.Threading;
using System.Collections.Generic;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio.VCCodeModel;
using Microsoft.VisualStudio.VCProjectEngine;
using Task = System.Threading.Tasks.Task;


namespace TabsManagerExtension.VsShell.Solution.Services {
    /// <summary>
    /// ExternalDependenciesMonitor
    /// </summary>
    public sealed class SolutionExternalDependenciesMonitorService :
        TabsManagerExtension.Services.SingletonServiceBase<SolutionExternalDependenciesMonitorService>,
        TabsManagerExtension.Services.IExtensionService {

        private readonly Dictionary<IVsHierarchy, VsShell.Project.ProjectExternalDependenciesTracker> _trackers = new();

        public event Action<_EventArgs.ProjectExternalDependenciesChangedEventArgs>? ExternalDependenciesChanged;


        public SolutionExternalDependenciesMonitorService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return new[] {
                typeof(VsShell.Services.VsIDEStateFlagsTrackerService),
                typeof(VsShell.Solution.Services.VsSolutionEventsTrackerService),
            };
        }


        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();
            Helpers.Diagnostic.Logger.LogDebug("[SolutionExternalDependenciesMonitorService] Initialized.");
        }


        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[SolutionExternalDependenciesMonitorService] Disposed.");
        }


        //
        // ░ API
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        public void AddProject(IVsHierarchy projectHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_trackers.ContainsKey(projectHierarchy)) {
                return;
            }

            var tracker = new VsShell.Project.ProjectExternalDependenciesTracker(projectHierarchy);
            tracker.ExternalDependenciesChanged += this.OnExternalDependenciesChanged;

            _trackers[projectHierarchy] = tracker;
        }

        public void RemoveProject(IVsHierarchy projectHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_trackers.TryGetValue(projectHierarchy, out var tracker)) {
                tracker.ExternalDependenciesChanged -= this.OnExternalDependenciesChanged;
                _trackers.Remove(projectHierarchy);
            }
        }


        //
        // ░ Event handlers
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
        private void OnExternalDependenciesChanged(_EventArgs.ProjectExternalDependenciesChangedEventArgs e) {
            this.ExternalDependenciesChanged?.Invoke(e);
        }


        //
        // ░ Internal logic
        // ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        //
    }


    //private void StartAnalyzeRoutineInBackground() {
    //        ThreadHelper.JoinableTaskFactory.RunAsync(async () => {
    //            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

    //            int attempts = 5;
    //            do {
    //                await Task.Delay(TimeSpan.FromSeconds(3));
    //                this.Analyze();
    //            } while (--attempts > 0);
    //        });
    //    }
    //}
}