using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Utilities;

namespace TabsManagerExtension.VsShell.Services {
    /// <summary>
    /// Универсальный трекер, подписывающийся на события выбора и активации окон в Visual Studio.
    /// Позволяет отслеживать:
    /// - смену активного окна (frame)
    /// - выбор элементов в Solution Explorer
    /// </summary>
   public sealed class VsSelectionTrackerService :
        TabsManagerExtension.Services.SingletonServiceBase<VsSelectionTrackerService>,
        TabsManagerExtension.Services.IExtensionService,
        IVsSelectionEvents {

        private readonly IVsMonitorSelection _monitorSelection;
        private readonly IVsUIShell _uiShell;
        private uint _cookie;

        public event Action<IVsWindowFrame>? VsWindowFrameActivated;
        public event Action<EnvDTE.ProjectItem>? ProjectItemSelected;

        public VsSelectionTrackerService() {
            ThreadHelper.ThrowIfNotOnUIThread();

            _monitorSelection = (IVsMonitorSelection)Package.GetGlobalService(typeof(SVsShellMonitorSelection))
                ?? throw new InvalidOperationException("Cannot get IVsMonitorSelection");

            _uiShell = (IVsUIShell)Package.GetGlobalService(typeof(SVsUIShell))
                ?? throw new InvalidOperationException("Cannot get IVsUIShell");

            int hr = _monitorSelection.AdviseSelectionEvents(this, out _cookie);
            ErrorHandler.ThrowOnFailure(hr);
        }

        public IReadOnlyList<Type> DependsOn() {
            return Array.Empty<Type>();
        }

        public void Initialize() {
            // Инициализация уже произошла в конструкторе, ничего не делаем.
            Helpers.Diagnostic.Logger.LogDebug("[VsSelectionTracker] Initialized.");
        }

        public void Shutdown() {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (_cookie != 0) {
                _monitorSelection.UnadviseSelectionEvents(_cookie);
                _cookie = 0;
            }

            ClearInstance();
            Helpers.Diagnostic.Logger.LogDebug("[VsSelectionTracker] Disposed.");
        }

        public int OnElementValueChanged(uint elementid, object oldValue, object newValue) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (elementid == (uint)VSConstants.VSSELELEMID.SEID_WindowFrame && newValue is IVsWindowFrame frame) {
                this.VsWindowFrameActivated?.Invoke(frame);
            }

            return VSConstants.S_OK;
        }

        public int OnSelectionChanged(
            IVsHierarchy pHierOld, uint itemidOld,
            IVsMultiItemSelect pMISOld, ISelectionContainer pSCOld,
            IVsHierarchy pHierNew, uint itemidNew,
            IVsMultiItemSelect pMISNew, ISelectionContainer pSCNew) {
            ThreadHelper.ThrowIfNotOnUIThread();

            if (pHierNew == null || itemidNew == VSConstants.VSITEMID_NIL) {
                return VSConstants.S_OK;
            }

            try {
                pHierNew.GetProperty(itemidNew, (int)__VSHPROPID.VSHPROPID_ExtObject, out object value);

                if (value is EnvDTE.ProjectItem projectItem) {
                    this.ProjectItemSelected?.Invoke(projectItem);
                }
            }
            catch (Exception ex) {
                Helpers.Diagnostic.Logger.LogError($"[VsSelectionTracker] OnSelectionChanged error: {ex}");
            }

            return VSConstants.S_OK;
        }

        public int OnCmdUIContextChanged(uint dwCmdUICookie, int fActive) => VSConstants.S_OK;
    }
}