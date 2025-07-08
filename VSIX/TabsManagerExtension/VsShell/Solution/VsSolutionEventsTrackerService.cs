using System;
using System.IO;
using System.Windows;
using System.Windows.Threading;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Utilities;


namespace TabsManagerExtension.VsShell.Solution.Services {
    /// <summary>
    /// Сервис отслеживания загрузки и выгрузки проектов через IVsSolutionEvents.
    /// </summary>
    public sealed class VsSolutionEventsTrackerService :
        TabsManagerExtension.Services.SingletonServiceBase<VsSolutionEventsTrackerService>,
        TabsManagerExtension.Services.IExtensionService,
        IVsSolutionEvents {

        public event Action<_EventArgs.ProjectLoadStateChangedEventArgs>? ProjectLoaded;
        public event Action<_EventArgs.ProjectLoadStateChangedEventArgs>? ProjectUnloaded;

        private IVsSolution? _vsSolution;
        private uint _cookie;

        public VsSolutionEventsTrackerService() { }

        //
        // IExtensionService
        //
        public IReadOnlyList<Type> DependsOn() {
            return Array.Empty<Type>();
        }

        public void Initialize() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _vsSolution = PackageServices.TryGetVsSolution();
            if (_vsSolution == null) {
                throw new InvalidOperationException("Cannot get SVsSolution");
            }

            ErrorHandler.ThrowOnFailure(_vsSolution.AdviseSolutionEvents(this, out _cookie));
            Helpers.Diagnostic.Logger.LogDebug("[VsSolutionEventsTrackerService] Initialized.");
        }

        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_vsSolution != null && _cookie != 0) {
                _vsSolution.UnadviseSolutionEvents(_cookie);
                _cookie = 0;
            }

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[VsSolutionEventsTrackerService] Disposed.");
        }

        //
        // IVsSolutionEvents
        //
        public int OnAfterOpenProject(IVsHierarchy pHierarchy, int fAdded) {
            return VSConstants.S_OK;
        }

        public int OnQueryCloseProject(IVsHierarchy pHierarchy, int fRemoving, ref int pfCancel) {
            return VSConstants.S_OK;
        }

        public int OnBeforeCloseProject(IVsHierarchy pHierarchy, int fRemoved) {
            return VSConstants.S_OK;
        }

        public int OnAfterLoadProject(IVsHierarchy pStubHierarchy, IVsHierarchy pRealHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(pRealHierarchy);
            this.ProjectLoaded?.Invoke(new _EventArgs.ProjectLoadStateChangedEventArgs(
                new VsShell.Hierarchy.VsStubHierarchy(pStubHierarchy),
                new VsShell.Hierarchy.VsRealHierarchy(pRealHierarchy),
                dteProject
                ));

            Helpers.Diagnostic.Logger.LogDebug($"[VsSolutionEventsTrackerService] Project loaded: {dteProject?.UniqueName}");
            return VSConstants.S_OK;
        }

        public int OnQueryUnloadProject(IVsHierarchy pRealHierarchy, ref int pfCancel) {
            return VSConstants.S_OK;
        }

        public int OnBeforeUnloadProject(IVsHierarchy pRealHierarchy, IVsHierarchy pStubHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(pRealHierarchy);
            this.ProjectUnloaded?.Invoke(new _EventArgs.ProjectLoadStateChangedEventArgs(
                new VsShell.Hierarchy.VsStubHierarchy(pStubHierarchy),
                new VsShell.Hierarchy.VsRealHierarchy(pRealHierarchy),
                dteProject
                ));

            Helpers.Diagnostic.Logger.LogDebug($"[VsSolutionEventsTrackerService] Project unloaded: {dteProject?.UniqueName}");
            return VSConstants.S_OK;
        }

        public int OnAfterOpenSolution(object pUnkReserved, int fNewSolution) {
            return VSConstants.S_OK;
        }

        public int OnQueryCloseSolution(object pUnkReserved, ref int pfCancel) {
            return VSConstants.S_OK;
        }

        public int OnBeforeCloseSolution(object pUnkReserved) {
            return VSConstants.S_OK;
        }

        public int OnAfterCloseSolution(object pUnkReserved) {
            return VSConstants.S_OK;
        }
    }
}