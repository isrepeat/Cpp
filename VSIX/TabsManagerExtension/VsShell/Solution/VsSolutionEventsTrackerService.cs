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
using EnvDTE;


namespace TabsManagerExtension.VsShell.Solution.Services {
    /// <summary>
    /// Сервис отслеживания загрузки и выгрузки проектов через IVsSolutionEvents.
    /// </summary>
    public sealed class VsSolutionEventsTrackerService :
        TabsManagerExtension.Services.SingletonServiceBase<VsSolutionEventsTrackerService>,
        TabsManagerExtension.Services.IExtensionService,
        IVsSolutionEvents {

        public event Action<_EventArgs.ProjectHierarchyChangedEventArgs>? ProjectLoaded;
        public event Action<_EventArgs.ProjectHierarchyChangedEventArgs>? ProjectUnloaded;

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
        /// <summary>
        /// Этот метод вызывается Visual Studio после открытия (добавления) проекта в Solution.
        /// <para>
        /// В отличие от OnAfterLoadProject, который вызывается только когда проект
        /// "переходит" из IVsStubHierarchy в IVsRealHierarchy (deferred load),
        /// OnAfterOpenProject вызывается в любых случаях:
        /// - при первом открытии проекта,
        /// - при загрузке ранее выгруженного проекта (если не было stub),
        /// - при открытии решения, содержащего этот проект,
        /// - при LoadProject из кода.
        /// </para>
        /// </summary>
        public int OnAfterOpenProject(IVsHierarchy pHierarchy, int fAdded) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (fAdded != 0) {
                var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(pHierarchy);
                Helpers.Diagnostic.Logger.LogDebug($"[VsSolutionEventsTrackerService] OnAfterOpenProject(): {dteProject?.UniqueName}");

                var pNewHierarchy = VsShell.Hierarchy.VsHierarchyFactory.CreateHierarchy(pHierarchy) as VsShell.Hierarchy.IVsRealHierarchy;
                if (pNewHierarchy != null) {
                    this.ProjectLoaded?.Invoke(new _EventArgs.ProjectHierarchyChangedEventArgs(
                        null, // oldHierarchy отсутствует, т.к. проект не переходил из stubHierarchy, а сразу загружен в Solution.
                        pNewHierarchy
                    ));
                }
                else {
                    // Это крайний случай: если Factory не смогла создать IVsRealHierarchy,
                    // возможно передан неподдерживаемый IVsHierarchy (например Misc Files).
                    Helpers.Diagnostic.Logger.LogWarning($"[VsSolutionEventsTrackerService] Could not cast hierarchy to IVsRealHierarchy.");
                }
            }
            return VSConstants.S_OK;
        }

        public int OnQueryCloseProject(IVsHierarchy pHierarchy, int fRemoving, ref int pfCancel) {
            return VSConstants.S_OK;
        }

        public int OnBeforeCloseProject(IVsHierarchy pHierarchy, int fRemoved) {
            return VSConstants.S_OK;
        }

        public int OnAfterLoadProject(IVsHierarchy pStubHierarchy, IVsHierarchy pRealHierarchy) {
            // Used OnAfterOpenProject instead.
            return VSConstants.S_OK;
        }

        public int OnQueryUnloadProject(IVsHierarchy pRealHierarchy, ref int pfCancel) {
            return VSConstants.S_OK;
        }

        public int OnBeforeUnloadProject(IVsHierarchy pRealHierarchy, IVsHierarchy pStubHierarchy) {
            ThreadHelper.ThrowIfNotOnUIThread();

            var dteProject = Utils.EnvDteUtils.GetDteProjectFromHierarchy(pRealHierarchy);
            Helpers.Diagnostic.Logger.LogDebug($"[VsSolutionEventsTrackerService] OnBeforeUnloadProject(): {dteProject?.UniqueName}");

            var pOldHierarchy = VsShell.Hierarchy.VsHierarchyFactory.CreateHierarchy(pRealHierarchy) as VsShell.Hierarchy.IVsRealHierarchy
                ?? throw new InvalidCastException("Expected IVsRealHierarchy but got different type.");

            var pNewHierarchy = VsShell.Hierarchy.VsHierarchyFactory.CreateHierarchy(pStubHierarchy) as VsShell.Hierarchy.IVsStubHierarchy
                ?? throw new InvalidCastException("Expected IVsStubHierarchy but got different type.");

            this.ProjectUnloaded?.Invoke(new _EventArgs.ProjectHierarchyChangedEventArgs(
                pOldHierarchy,
                pNewHierarchy
                ));

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